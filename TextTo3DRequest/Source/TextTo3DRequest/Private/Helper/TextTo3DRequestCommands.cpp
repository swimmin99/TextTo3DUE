// Copyright 2025 Devhanghae All Rights Reserved.
#include "Helper/TextTo3DRequestCommands.h"

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "TextTo3DRequestCommands"

void FTextTo3DRequestCommands::RegisterCommands()
{
    UI_COMMAND(
        OpenPluginWindow,
        "Shap-E Generator", 
        "Opens the Shap-E Text-to-3D Generator window.", 
        EUserInterfaceActionType::Button, 
        FInputChord()
    );
}

#undef LOCTEXT_NAMESPACE

#endif