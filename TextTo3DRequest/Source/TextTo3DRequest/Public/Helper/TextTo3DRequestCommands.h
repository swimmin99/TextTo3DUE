// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "Framework/Commands/Commands.h" 
#include "Styling/AppStyle.h"

/**
 * Defines the UI commands for the TextTo3DRequest plugin.
 */
class FTextTo3DRequestCommands : public TCommands<FTextTo3DRequestCommands>
{
public:
    FTextTo3DRequestCommands()
        : TCommands<FTextTo3DRequestCommands>(
            TEXT("TextTo3DRequest"),
            NSLOCTEXT("Contexts", "TextTo3DRequest", "TextTo3DRequest Plugin"), 
            NAME_None,
            FAppStyle::GetAppStyleSetName() 
        )
    {
    }

    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> OpenPluginWindow;

};

#endif