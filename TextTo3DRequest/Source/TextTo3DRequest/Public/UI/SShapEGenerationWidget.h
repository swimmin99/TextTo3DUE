// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR

#include "Widgets/SCompoundWidget.h"
#include "Manager/FShapEProcessManager.h"

class SEditableTextBox;
class SButton;
template <typename NumericType> class SSpinBox;
class SCheckBox;
class SProgressBar;
class STextBlock;
class SScrollBox;

class SShapEGenerationWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SShapEGenerationWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SShapEGenerationWidget() override;

private:
    TSharedPtr<SEditableTextBox> BatFilePathTextBox;
    TSharedPtr<SEditableTextBox> PromptTextBox;
    TSharedPtr<SEditableTextBox> OutputDirTextBox;
    TSharedPtr<SSpinBox<float>> GuidanceScaleSpinBox;
    TSharedPtr<SSpinBox<int32>> KarrasStepsSpinBox;
    TSharedPtr<SCheckBox> UseFP16CheckBox;
    TSharedPtr<SButton> GenerateButton;
    TSharedPtr<SButton> ActionButton;

    TSharedPtr<SProgressBar> ProgressBar;
    TSharedPtr<STextBlock> StatusTextBlock;
    TSharedPtr<STextBlock> LogTextBlock;
    TSharedPtr<SScrollBox> LogScrollBox;

    TSharedPtr<FShapEProcessManager> ProcessManager;

    // Default Path
    FString CurrentBatFilePath = TEXT("C:/AIModel/shap-e-local/run_shape.bat");
    FString CurrentOutputDir = TEXT("D:/UP/P/Customizing/Content/Characters");

    // Cond Var
    bool bIsGenerationFinished = false;
    bool bWasCanceled = false;

    FReply OnBrowseBatFileClicked();
    FReply OnBrowseOutputDirClicked();
    FReply OnGenerateButtonClicked();
    FReply OnActionButtonClicked();

    void HandleProgressUpdated(float Percentage, int32 Step, int32 TotalSteps, const FString& RawMessage);
    void HandleStatusMessageReceived(const FString& Message);
    void HandleGenerationComplete(const FString& PlyPath, const FString& ObjPath, const FString& RawMessage);
    void HandleErrorReceived(const FString& ErrorMessage, const FString& ErrorType, const FString& RawMessage);
    void HandleInfoMessageReceived(const FString& Message);
    void HandleProcessFinished();

    void AddLogMessage(const FString& Message, const FLinearColor& Color = FLinearColor::White);
    void ResetUIState();

    bool IsGenerateButtonEnabled() const;
    EVisibility GetActionButtonVisibility() const;
    FText GetActionButtonText() const;
};

#endif