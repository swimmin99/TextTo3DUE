// Copyright 2025 Devhanghae All Rights Reserved.
#include "UI/SShapEGenerationWidget.h"
#if WITH_EDITOR
#include "TextTo3DRequest.h"
#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"

void SShapEGenerationWidget::Construct(const FArguments& InArgs)
{
    FTextTo3DRequestModule& Module = FModuleManager::LoadModuleChecked<FTextTo3DRequestModule>("TextTo3DRequest");
    ProcessManager = Module.GetProcessManager();

    if (!ProcessManager.IsValid())
    {
        ChildSlot
            [
                SNew(STextBlock)
                    .Text(FText::FromString(TEXT("FATAL ERROR: ShapEProcessManager is unavailable!")))
                    .ColorAndOpacity(FSlateColor(FLinearColor::Red))
                    .Justification(ETextJustify::Center)
            ];
        UE_LOG(LogTemp, Error, TEXT("SShapEGenerationWidget: FShapEProcessManager is not valid."));
        return;
    }

    ProcessManager->OnProgressUpdated().AddSP(this, &SShapEGenerationWidget::HandleProgressUpdated);
    ProcessManager->OnStatusMessageReceived().AddSP(this, &SShapEGenerationWidget::HandleStatusMessageReceived);
    ProcessManager->OnGenerationComplete().AddSP(this, &SShapEGenerationWidget::HandleGenerationComplete);
    ProcessManager->OnErrorReceived().AddSP(this, &SShapEGenerationWidget::HandleErrorReceived);
    ProcessManager->OnInfoMessageReceived().AddSP(this, &SShapEGenerationWidget::HandleInfoMessageReceived);
    ProcessManager->OnProcessFinished().AddSP(this, &SShapEGenerationWidget::HandleProcessFinished);

    ChildSlot
        [
            SNew(SVerticalBox)

                // UI for Setting Batch Directory
                + SVerticalBox::Slot().AutoHeight().Padding(2, 2)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().VAlign(VAlign_Center).Padding(0, 0, 5, 0)[SNew(STextBlock).Text(FText::FromString(TEXT("Batch File:")))]
                        + SHorizontalBox::Slot().FillWidth(1.0f)[SAssignNew(BatFilePathTextBox, SEditableTextBox).Text(FText::FromString(CurrentBatFilePath)).HintText(FText::FromString(TEXT("Path to run_shape.bat"))).OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type) { CurrentBatFilePath = NewText.ToString(); })]
                        + SHorizontalBox::Slot().AutoWidth().Padding(2, 0, 0, 0)[SNew(SButton).Text(FText::FromString(TEXT("Browse..."))).OnClicked(this, &SShapEGenerationWidget::OnBrowseBatFileClicked)]
                ]
                // UI for setting Prompt
                + SVerticalBox::Slot().AutoHeight().Padding(2, 5)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().VAlign(VAlign_Center).Padding(0, 0, 5, 0)[SNew(STextBlock).Text(FText::FromString(TEXT("Prompt:")))]
                        + SHorizontalBox::Slot().FillWidth(1.0f)[SAssignNew(PromptTextBox, SEditableTextBox).HintText(FText::FromString(TEXT("Enter text prompt (e.g., a cowboy hat)")))]
                ]
                // UI for setting Output Directory
                + SVerticalBox::Slot().AutoHeight().Padding(2, 2)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().VAlign(VAlign_Center).Padding(0, 0, 5, 0)[SNew(STextBlock).Text(FText::FromString(TEXT("Output Directory:")))]
                        + SHorizontalBox::Slot().FillWidth(1.0f)[SAssignNew(OutputDirTextBox, SEditableTextBox).Text(FText::FromString(CurrentOutputDir)).HintText(FText::FromString(TEXT("Directory to save generated meshes"))).OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type) { CurrentOutputDir = NewText.ToString(); })]
                        + SHorizontalBox::Slot().AutoWidth().Padding(2, 0, 0, 0)[SNew(SButton).Text(FText::FromString(TEXT("Browse..."))).OnClicked(this, &SShapEGenerationWidget::OnBrowseOutputDirClicked)]
                ]
                // UI for ai model params
                + SVerticalBox::Slot().AutoHeight().Padding(2, 5)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 5, 0).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::FromString(TEXT("Guidance Scale:")))]
                        + SHorizontalBox::Slot().AutoWidth()[SAssignNew(GuidanceScaleSpinBox, SSpinBox<float>).MinValue(1.0f).MaxValue(30.0f).Value(15.0f).Delta(0.1f)]
                        + SHorizontalBox::Slot().AutoWidth().Padding(10, 0, 5, 0).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::FromString(TEXT("Karras Steps:")))]
                        + SHorizontalBox::Slot().AutoWidth()[SAssignNew(KarrasStepsSpinBox, SSpinBox<int32>).MinValue(16).MaxValue(256).Value(16).Delta(1)]
                        + SHorizontalBox::Slot().AutoWidth().Padding(10, 0, 0, 0).VAlign(VAlign_Center)[SAssignNew(UseFP16CheckBox, SCheckBox).IsChecked(ECheckBoxState::Checked)[SNew(STextBlock).Text(FText::FromString(TEXT("Use FP16")))]]
                ]
                // UI for Generate Model
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(5, 10)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(5)
                        [
                            SAssignNew(GenerateButton, SButton)
                                .Text(FText::FromString(TEXT("Generate Model")))
                                .OnClicked(this, &SShapEGenerationWidget::OnGenerateButtonClicked)
                                .IsEnabled(this, &SShapEGenerationWidget::IsGenerateButtonEnabled)
                        ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(5)
                        [
                            SAssignNew(ActionButton, SButton)
                                .OnClicked(this, &SShapEGenerationWidget::OnActionButtonClicked)
                                .Text(this, &SShapEGenerationWidget::GetActionButtonText)
                                .Visibility(this, &SShapEGenerationWidget::GetActionButtonVisibility)
                        ]
                ]
                // UI for progress bar
                + SVerticalBox::Slot().AutoHeight().Padding(2, 5)
                [
                    SAssignNew(ProgressBar, SProgressBar).Percent(0.0f)
                ]
                // UI for displaying status and log
                + SVerticalBox::Slot().AutoHeight().Padding(2, 2)
                [
                    SAssignNew(StatusTextBlock, STextBlock).Text(FText::FromString(TEXT("Idle")))
                ]
                + SVerticalBox::Slot().FillHeight(1.0f).Padding(2, 5)
                [
                    SNew(SBorder).Padding(FMargin(3))
                        [
                            SAssignNew(LogScrollBox, SScrollBox)
                                + SScrollBox::Slot()
                                [
                                    SAssignNew(LogTextBlock, STextBlock).AutoWrapText(true)
                                ]
                        ]
                ]
        ];
}

SShapEGenerationWidget::~SShapEGenerationWidget()
{
    if (ProcessManager.IsValid())
    {
        ProcessManager->OnProgressUpdated().RemoveAll(this);
        ProcessManager->OnStatusMessageReceived().RemoveAll(this);
        ProcessManager->OnGenerationComplete().RemoveAll(this);
        ProcessManager->OnErrorReceived().RemoveAll(this);
        ProcessManager->OnInfoMessageReceived().RemoveAll(this);
        ProcessManager->OnProcessFinished().RemoveAll(this);
    }
}

FReply SShapEGenerationWidget::OnBrowseBatFileClicked()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        TArray<FString> OutFiles;
        if (DesktopPlatform->OpenFileDialog(
            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
            TEXT("Select run_shape.bat"), TEXT(""), TEXT(""),
            TEXT("Batch Files (*.bat)|*.bat|All Files (*.*)|*.*"),
            EFileDialogFlags::None, OutFiles))
        {
            if (OutFiles.Num() > 0)
            {
                CurrentBatFilePath = OutFiles[0];
                BatFilePathTextBox->SetText(FText::FromString(CurrentBatFilePath));
            }
        }
    }
    return FReply::Handled();
}

FReply SShapEGenerationWidget::OnBrowseOutputDirClicked()
{
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        FString OutFolderName;
        if (DesktopPlatform->OpenDirectoryDialog(
            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
            TEXT("Select Output Directory"), TEXT(""), OutFolderName))
        {
            CurrentOutputDir = OutFolderName;
            OutputDirTextBox->SetText(FText::FromString(CurrentOutputDir));
        }
    }
    return FReply::Handled();
}

FReply SShapEGenerationWidget::OnGenerateButtonClicked()
{
    if (!ProcessManager.IsValid() || ProcessManager->IsRunning())
    {
        return FReply::Handled();
    }

    FString BatFilePath = BatFilePathTextBox->GetText().ToString();
    FString Prompt = PromptTextBox->GetText().ToString();

    if (BatFilePath.IsEmpty() || !FPaths::FileExists(BatFilePath))
    {
        AddLogMessage(TEXT("Error: Batch file path is invalid."), FLinearColor::Red);
        return FReply::Handled();
    }
    if (Prompt.IsEmpty())
    {
        AddLogMessage(TEXT("Error: Prompt cannot be empty."), FLinearColor::Red);
        return FReply::Handled();
    }

    bIsGenerationFinished = false;
    bWasCanceled = false;

    FShapEGenerationParameters Params;
    Params.Prompt = Prompt;
    Params.OutputDirectory = OutputDirTextBox->GetText().ToString();
    Params.GuidanceScale = GuidanceScaleSpinBox->GetValue();
    Params.KarrasSteps = KarrasStepsSpinBox->GetValue();
    Params.bUseFP16 = UseFP16CheckBox->IsChecked();

    LogTextBlock->SetText(FText::GetEmpty());
    AddLogMessage(TEXT("Starting generation process..."), FLinearColor(0.8f, 0.8f, 1.0f));
    ProgressBar->SetPercent(0.0f);
    StatusTextBlock->SetText(FText::FromString(TEXT("Initializing...")));

    if (!ProcessManager->LaunchProcess(BatFilePath, Params))
    {
        AddLogMessage(TEXT("Error: Failed to launch process. Check path and log for details."), FLinearColor::Red);
        HandleProcessFinished();
    }

    return FReply::Handled();
}

FReply SShapEGenerationWidget::OnActionButtonClicked()
{
    if (!ProcessManager.IsValid()) return FReply::Handled();

    if (ProcessManager->IsRunning())
    {
        bWasCanceled = true;
        AddLogMessage(TEXT("Cancellation requested by user..."), FLinearColor::Yellow);
        ProcessManager->RequestStopProcess();
    }
    else if (bIsGenerationFinished)
    {
        ResetUIState();
    }

    return FReply::Handled();
}

void SShapEGenerationWidget::HandleProgressUpdated(float Percentage, int32 Step, int32 TotalSteps, const FString& RawMessage)
{
    ProgressBar->SetPercent(Percentage / 100.0f);

    if (Step > 0 && TotalSteps > 0)
    {
        StatusTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Generating... Step %d / %d (%.0f%%)"), Step, TotalSteps, Percentage)));
    }
    else
    {
        StatusTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Processing... (%.0f%%)"), Percentage)));
    }
}

void SShapEGenerationWidget::HandleStatusMessageReceived(const FString& Message)
{
    StatusTextBlock->SetText(FText::FromString(Message));
    AddLogMessage(FString::Printf(TEXT("[STATUS] %s"), *Message));
}

void SShapEGenerationWidget::HandleGenerationComplete(const FString& PlyPath, const FString& ObjPath, const FString& RawMessage)
{
    ProgressBar->SetPercent(1.0f);
    FString CompleteMsg = FString::Printf(TEXT("Generation Complete! Files saved.\nPLY: %s\nOBJ: %s"), *PlyPath, *ObjPath);
    StatusTextBlock->SetText(FText::FromString(TEXT("Generation Complete!")));
    AddLogMessage(CompleteMsg, FLinearColor::Green);
    bIsGenerationFinished = true;
}

void SShapEGenerationWidget::HandleErrorReceived(const FString& ErrorMessage, const FString& ErrorType, const FString& RawMessage)
{
    ProgressBar->SetPercent(0.0f);
    FString FullErrorMsg = FString::Printf(TEXT("ERROR (%s): %s"), *ErrorType, *ErrorMessage);
    StatusTextBlock->SetText(FText::FromString(TEXT("Error Occurred!")));
    AddLogMessage(FullErrorMsg, FLinearColor::Red);
    if (!RawMessage.IsEmpty() && !RawMessage.Contains(ErrorMessage))
    {
        AddLogMessage(FString::Printf(TEXT("Raw Data: %s"), *RawMessage), FLinearColor(0.8f, 0.2f, 0.2f));
    }
    bIsGenerationFinished = true;
}

void SShapEGenerationWidget::HandleInfoMessageReceived(const FString& Message)
{
    AddLogMessage(FString::Printf(TEXT("[INFO] %s"), *Message), FLinearColor(0.6f, 0.6f, 0.6f));
}

void SShapEGenerationWidget::HandleProcessFinished()
{
    if (bWasCanceled)
    {
        AddLogMessage(TEXT("Process has been canceled."), FLinearColor::Yellow);
        ResetUIState();
    }
    else
    {
        bIsGenerationFinished = true;
    }
}

void SShapEGenerationWidget::AddLogMessage(const FString& Message, const FLinearColor& Color)
{
    if (LogTextBlock.IsValid() && LogScrollBox.IsValid())
    {
        FString Timestamp = FDateTime::Now().ToString(TEXT("[%H:%M:%S] "));
        FString CurrentLog = LogTextBlock->GetText().ToString();
        if (!CurrentLog.IsEmpty())
        {
            CurrentLog += TEXT("\n");
        }
        CurrentLog += Timestamp + Message;
        LogTextBlock->SetText(FText::FromString(CurrentLog));
        LogScrollBox->ScrollToEnd();
    }
}

void SShapEGenerationWidget::ResetUIState()
{
    bIsGenerationFinished = false;
    bWasCanceled = false;
    ProgressBar->SetPercent(0.0f);
    StatusTextBlock->SetText(FText::FromString(TEXT("Idle")));
    LogTextBlock->SetText(FText::GetEmpty());
}

bool SShapEGenerationWidget::IsGenerateButtonEnabled() const
{
    return ProcessManager.IsValid() && !ProcessManager->IsRunning() && !bIsGenerationFinished;
}

EVisibility SShapEGenerationWidget::GetActionButtonVisibility() const
{
    if (ProcessManager.IsValid() && (ProcessManager->IsRunning() || bIsGenerationFinished))
    {
        return EVisibility::Visible;
    }
    return EVisibility::Collapsed;
}

FText SShapEGenerationWidget::GetActionButtonText() const
{
    if (ProcessManager.IsValid() && ProcessManager->IsRunning())
    {
        return FText::FromString(TEXT("Cancel"));
    }
    else if (bIsGenerationFinished)
    {
        return FText::FromString(TEXT("Done"));
    }
    return FText::GetEmpty();
}

#endif