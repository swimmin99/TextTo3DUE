// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Manager/FShapEProcessManager.h"

#if WITH_EDITOR
class FUICommandList;
#endif

class FTextTo3DRequestModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static inline FTextTo3DRequestModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FTextTo3DRequestModule>("TextTo3DRequest");
    }

    TSharedPtr<FShapEProcessManager> GetProcessManager() const
    {
        return ShapEProcessManager;
    }

private:
    TSharedPtr<FShapEProcessManager> ShapEProcessManager;

#if WITH_EDITOR 
    void RegisterMenus();
    void PluginButtonClicked();
    TSharedPtr<FUICommandList> PluginCommands;
#endif
};