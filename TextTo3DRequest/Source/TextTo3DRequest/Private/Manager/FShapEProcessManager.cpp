// Copyright 2025 Devhanghae All Rights Reserved.
#include "Manager/FShapEProcessManager.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Async/Async.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Base64.h"


FString FShapEGenerationParameters::ToJsonString() const
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("prompt"), Prompt);
    JsonObject->SetStringField(TEXT("output_dir"), OutputDirectory);
    JsonObject->SetNumberField(TEXT("guidance_scale"), GuidanceScale);
    JsonObject->SetNumberField(TEXT("karras_steps"), KarrasSteps);
    JsonObject->SetBoolField(TEXT("use_fp16"), bUseFP16);

    FString OutputString;
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer, true);

    
    return OutputString;
}

FShapEProcessManager::~FShapEProcessManager()
{
    RequestStopProcess();
    if (ReaderThread)
    {
        ReaderThread->WaitForCompletion();
        delete ReaderThread;
        ReaderThread = nullptr;
    }
    OutputReaderRunnable.Reset();
    CleanupProcessHandles();
}

bool FShapEProcessManager::LaunchProcess(const FString& ScriptPath, const FShapEGenerationParameters& Params)
{
    FScopeLock Lock(&ProcessManagementCS);

    if (bIsProcessRunning)
    {
        AsyncTask(ENamedThreads::GameThread, [this]() {
            ErrorReceivedDelegate.Broadcast(TEXT("Another generation process is already running."), TEXT("ProcessBusy"), TEXT(""));
            });
        return false;
    }

    const FString BatPath = FPaths::Combine(FPaths::GetPath(ScriptPath), TEXT("run_shape.bat"));
    if (!FPaths::FileExists(BatPath))
    {
        const FString ErrorMsg = FString::Printf(TEXT("Batch file not found: %s"), *BatPath);
        UE_LOG(LogTemp, Error, TEXT("FShapEProcessManager: %s"), *ErrorMsg);
        AsyncTask(ENamedThreads::GameThread, [this, ErrorMsg]() {
            ErrorReceivedDelegate.Broadcast(ErrorMsg, TEXT("FileNotFound"), TEXT(""));
            });
        return false;
    }

    CleanupProcessHandles();

    if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
    {
        AsyncTask(ENamedThreads::GameThread, [this]() {
            ErrorReceivedDelegate.Broadcast(TEXT("Failed to create stdout pipe for batch process."), TEXT("PipeError"), TEXT(""));
            });
        return false;
    }

    const FString JsonParamsString = Params.ToJsonString();
    const FString Base64Params = FBase64::Encode(JsonParamsString);

    // ue flag to skip batch file to skip directory setting process
    const FString CommandLineArgs = FString::Printf(TEXT("--ue --params-base64 \"%s\""), *Base64Params);

    const FString WorkingDirectory = FPaths::GetPath(BatPath);

    PythonProcessHandle = FPlatformProcess::CreateProc(
        *BatPath,
        *CommandLineArgs,
        false,    // bLaunchDetached
        true,     // bLaunchHidden
        true,     // bLaunchReallyHidden
        nullptr,  // OutProcessID
        0,        // PriorityModifier
        *WorkingDirectory,
        WritePipe, // Process's StdOut
        nullptr    // Process's StdIn -> Not using
    );

    if (!PythonProcessHandle.IsValid())
    {
        AsyncTask(ENamedThreads::GameThread, [this]() {
            ErrorReceivedDelegate.Broadcast(TEXT("Failed to launch batch file. Check permissions and paths."), TEXT("ProcessLaunchError"), TEXT(""));
            });
        CleanupProcessHandles();
        return false;
    }

    bIsProcessRunning = true;

    // stdout, start read thread
    OutputReaderRunnable = MakeShared<FShapEOutputReaderRunnable>(ReadPipe, StaticCastSharedRef<FShapEProcessManager>(AsShared()));
    ReaderThread = FRunnableThread::Create(OutputReaderRunnable.Get(), TEXT("ShapEOutputReaderThread"));
    if (!ReaderThread)
    {
        UE_LOG(LogTemp, Error, TEXT("FShapEProcessManager: Failed to create output reader thread."));
        RequestStopProcess();
        AsyncTask(ENamedThreads::GameThread, [this]() {
            ErrorReceivedDelegate.Broadcast(TEXT("Failed to create reader thread."), TEXT("ThreadError"), TEXT(""));
            });
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("FShapEProcessManager: Batch file launched successfully with args: %s"), *CommandLineArgs);
    return true;
}

void FShapEProcessManager::RequestStopProcess()
{
    bool bWasRunning = false;
    {
        FScopeLock Lock(&ProcessManagementCS);

        if (!bIsProcessRunning && !PythonProcessHandle.IsValid())
        {
            return;
        }


        if (OutputReaderRunnable.IsValid())
        {
            OutputReaderRunnable->Stop();
        }

        if (PythonProcessHandle.IsValid())
        {
            if (FPlatformProcess::IsProcRunning(PythonProcessHandle))
            {
                FPlatformProcess::TerminateProc(PythonProcessHandle, true);
            }
            FPlatformProcess::CloseProc(PythonProcessHandle);
            PythonProcessHandle.Reset();
        }

        CleanupProcessHandles();

        if (bIsProcessRunning)
        {
            bIsProcessRunning = false;
            bWasRunning = true;
        }
    }

    if (bWasRunning)
    {
        NotifyProcessFinished();
    }
}

bool FShapEProcessManager::IsRunning()
{
    FScopeLock Lock(&ProcessManagementCS);
    return bIsProcessRunning && PythonProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(PythonProcessHandle);
}

void FShapEProcessManager::CleanupProcessHandles()
{
    // closes write pipe (used by child process) - read pipe closing is handled by OutputReaderRunnable
    if (WritePipe)
    {
        FPlatformProcess::ClosePipe(0, WritePipe);
        WritePipe = nullptr;
    }
}

void FShapEProcessManager::HandlePythonOutputLine(const FString& OutputLine)
{
    if (OutputLine.IsEmpty()) return;

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutputLine);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString Type;
        if (JsonObject->TryGetStringField(TEXT("type"), Type))
        {
            if (Type == TEXT("status"))
            {
                FString Message = JsonObject->GetStringField(TEXT("message"));
                AsyncTask(ENamedThreads::GameThread, [this, Message, OutputLine]() {
                    StatusMessageReceivedDelegate.Broadcast(Message);
                    if (Message.Contains(TEXT("Loading models"))) ProgressUpdatedDelegate.Broadcast(1.f, 0, 0, OutputLine);
                    else if (Message.Contains(TEXT("Models loaded"))) ProgressUpdatedDelegate.Broadcast(10.f, 0, 0, OutputLine);
                    else if (Message.Contains(TEXT("Decoding latents"))) ProgressUpdatedDelegate.Broadcast(95.f, 0, 0, OutputLine);
                    });
            }
            else if (Type == TEXT("complete"))
            {
                FString PlyPath = JsonObject->GetStringField(TEXT("ply_file"));
                FString ObjPath = JsonObject->GetStringField(TEXT("obj_file"));
                AsyncTask(ENamedThreads::GameThread, [this, PlyPath, ObjPath, OutputLine]() {
                    FScopeLock Lock(&ProcessManagementCS);
                    bIsProcessRunning = false;
                    ProgressUpdatedDelegate.Broadcast(100.f, 0, 0, OutputLine);
                    GenerationCompleteDelegate.Broadcast(PlyPath, ObjPath, OutputLine);
                    NotifyProcessFinished();
                    });
            }
            else if (Type == TEXT("error"))
            {
                FString Message = JsonObject->GetStringField(TEXT("message"));
                FString ErrorType = JsonObject->GetStringField(TEXT("error_type"));
                AsyncTask(ENamedThreads::GameThread, [this, Message, ErrorType, OutputLine]() {
                    FScopeLock Lock(&ProcessManagementCS);
                    bIsProcessRunning = false;
                    ErrorReceivedDelegate.Broadcast(Message, ErrorType, OutputLine);
                    NotifyProcessFinished();
                    });
            }
            else if (Type == TEXT("info") || Type == TEXT("debug"))
            {
                FString Message = JsonObject->GetStringField(TEXT("message"));
                AsyncTask(ENamedThreads::GameThread, [this, Message]() {
                    InfoMessageReceivedDelegate.Broadcast(Message);
                    });
            }
        }
    }
    else
    {
        FString CleanedLine = OutputLine.Replace(TEXT("\r"), TEXT("")).TrimStartAndEnd();

        // Process of parsing progress output from python
        if (CleanedLine.Contains(TEXT("%|")) && (CleanedLine.Contains(TEXT("s/it")) || CleanedLine.Contains(TEXT("it/s"))))
        {
            UE_LOG(LogTemp, Log, TEXT("HandlePythonOutputLine: Line identified as tqdm format: '%s'"), *CleanedLine);

            int32 PercentageCharIndex;
            if (CleanedLine.FindChar(TEXT('%'), PercentageCharIndex))
            {
                FString LeftSide = CleanedLine.Left(PercentageCharIndex);
                FString PercentageStr;

                for (int i = LeftSide.Len() - 1; i >= 0; --i)
                {
                    if (FChar::IsDigit(LeftSide[i]))
                    {
                        PercentageStr.InsertAt(0, LeftSide[i]);
                    }
                    else
                    {
                        if (!PercentageStr.IsEmpty()) break;
                    }
                }

                UE_LOG(LogTemp, Log, TEXT("HandlePythonOutputLine: Parsed percentage string is '%s'"), *PercentageStr);

                if (PercentageStr.IsNumeric())
                {
                    const int32 TqdmPercentage = FCString::Atoi(*PercentageStr);
                    const float MappedPercentage = 10.f + (TqdmPercentage / 100.f) * 85.f;

                    UE_LOG(LogTemp, Warning, TEXT(">>> SUCCESS: Broadcasting Progress: %.2f%% (from tqdm %d%%)"), MappedPercentage, TqdmPercentage);

                    AsyncTask(ENamedThreads::GameThread, [this, MappedPercentage, CleanedLine]() {
                        ProgressUpdatedDelegate.Broadcast(MappedPercentage, 0, 0, CleanedLine);
                        });
                    return;
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("FShapEProcessManager: Received non-JSON, non-tqdm output: %s"), *OutputLine);
    }
}



void FShapEProcessManager::NotifyProcessFinished()
{
    AsyncTask(ENamedThreads::GameThread, [this]() {
        ProcessFinishedDelegate.Broadcast();
        });
}


//=====================================================================================================\\
// --- FShapEOutputReaderRunnable Implementation ---

FShapEOutputReaderRunnable::FShapEOutputReaderRunnable(void* InReadPipe, TSharedRef<FShapEProcessManager, ESPMode::ThreadSafe> InProcessManager)
    : ReadPipe(InReadPipe)
    , ProcessManagerPtr(InProcessManager)
    , bStopRequested(false)
    , bFinished(false)
{
}

FShapEOutputReaderRunnable::~FShapEOutputReaderRunnable()
{
    if (ReadPipe)
    {
        FPlatformProcess::ClosePipe(ReadPipe, nullptr);
        ReadPipe = nullptr;
    }
}

bool FShapEOutputReaderRunnable::Init()
{
    bFinished = false;
    return ReadPipe != nullptr;
}

uint32 FShapEOutputReaderRunnable::Run()
{
    if (!ReadPipe)
    {
        bFinished = true;
        return 1;
    }

    while (!bStopRequested)
    {
        FString Output = FPlatformProcess::ReadPipe(ReadPipe);

        if (!Output.IsEmpty())
        {
            TArray<FString> Lines;
            Output.ParseIntoArrayLines(Lines, false); 
            for (const FString& Line : Lines)
            {
                if (!Line.TrimStartAndEnd().IsEmpty())
                {
                    if (TSharedPtr<FShapEProcessManager> ProcManager = ProcessManagerPtr.Pin())
                    {
                        ProcManager->HandlePythonOutputLine(Line);
                    }
                    else
                    {
                        bStopRequested = true;
                        break;
                    }
                }
            }
        }

        if (TSharedPtr<FShapEProcessManager> ProcManager = ProcessManagerPtr.Pin())
        {
            if (!ProcManager->IsRunning())
            {
                break;
            }
        }
        else
        {
            break; 
        }

        FPlatformProcess::Sleep(0.02f);
    }

    bFinished = true;

    if (TSharedPtr<FShapEProcessManager> ProcManager = ProcessManagerPtr.Pin())
    {
        AsyncTask(ENamedThreads::GameThread, [ProcManager]() {
            FScopeLock Lock(&(ProcManager->ProcessManagementCS));
            if (ProcManager->bIsProcessRunning)
            {
                ProcManager->bIsProcessRunning = false;
                ProcManager->NotifyProcessFinished();
            }
            });
    }

    if (ReadPipe)
    {
        FPlatformProcess::ClosePipe(ReadPipe, nullptr);
        ReadPipe = nullptr;
    }

    return 0;
}

void FShapEOutputReaderRunnable::Stop()
{
    bStopRequested = true;
}
