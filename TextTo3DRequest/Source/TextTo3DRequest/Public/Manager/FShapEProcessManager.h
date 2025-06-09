// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeBool.h" 
#include "Delegates/DelegateCombinations.h"

struct FShapEGenerationParameters
{
    FString Prompt;
    FString OutputDirectory;
    float GuidanceScale = 15.0f;
    int32 KarrasSteps = 64;
    bool bUseFP16 = true;

    FString ToJsonString() const; 
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnShapEProgressUpdated, float /*Percentage*/, int32 /*Step*/, int32 /*TotalSteps*/, const FString& /*RawMessage*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShapEStatusMessageReceived, const FString& /*Message*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnShapEGenerationComplete, const FString& /*PlyPath*/, const FString& /*ObjPath*/, const FString& /*RawMessage*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnShapEErrorReceived, const FString& /*ErrorMessage*/, const FString& /*ErrorType*/, const FString& /*RawMessage*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShapEInfoMessageReceived, const FString& /*Message*/);
DECLARE_MULTICAST_DELEGATE(FOnShapEProcessFinished);

class FShapEOutputReaderRunnable; 

class FShapEProcessManager : public TSharedFromThis<FShapEProcessManager>
{
public:
    ~FShapEProcessManager();

    bool LaunchProcess(const FString& ScriptPath, const FShapEGenerationParameters& Params);
    void RequestStopProcess();
    bool IsRunning();

    FOnShapEProgressUpdated& OnProgressUpdated() { return ProgressUpdatedDelegate; }
    FOnShapEStatusMessageReceived& OnStatusMessageReceived() { return StatusMessageReceivedDelegate; }
    FOnShapEGenerationComplete& OnGenerationComplete() { return GenerationCompleteDelegate; }
    FOnShapEErrorReceived& OnErrorReceived() { return ErrorReceivedDelegate; }
    FOnShapEInfoMessageReceived& OnInfoMessageReceived() { return InfoMessageReceivedDelegate; }
    FOnShapEProcessFinished& OnProcessFinished() { return ProcessFinishedDelegate; }


private:
    friend class FShapEOutputReaderRunnable;

    // Process and Pipe handles
    FProcHandle PythonProcessHandle;
    void* ReadPipe = nullptr;  // pipe for reading stdout of child process
    void* WritePipe = nullptr; // handle for child process to write

    // Asynch
    FRunnableThread* ReaderThread = nullptr;
    TSharedPtr<FShapEOutputReaderRunnable> OutputReaderRunnable;

    // condVar
    FCriticalSection ProcessManagementCS;
    bool bIsProcessRunning = false;

    void CleanupProcessHandles();
    void HandlePythonOutputLine(const FString& OutputLine);
    void NotifyProcessFinished();

    FOnShapEProgressUpdated ProgressUpdatedDelegate;
    FOnShapEStatusMessageReceived StatusMessageReceivedDelegate;
    FOnShapEGenerationComplete GenerationCompleteDelegate;
    FOnShapEErrorReceived ErrorReceivedDelegate;
    FOnShapEInfoMessageReceived InfoMessageReceivedDelegate;
    FOnShapEProcessFinished ProcessFinishedDelegate;
};

// Runnable Class for Reading Async
class FShapEOutputReaderRunnable : public FRunnable
{
public:
    FShapEOutputReaderRunnable(void* InReadPipe, TSharedRef<FShapEProcessManager, ESPMode::ThreadSafe> InProcessManager);
    virtual ~FShapEOutputReaderRunnable();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

    bool IsFinished() const { return bFinished; }

private:
    void* ReadPipe = nullptr;
    TWeakPtr<FShapEProcessManager> ProcessManagerPtr;
    FThreadSafeBool bStopRequested;
    FThreadSafeBool bFinished;
};