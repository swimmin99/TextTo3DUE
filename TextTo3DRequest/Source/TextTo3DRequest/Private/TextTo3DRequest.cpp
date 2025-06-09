// Copyright 2025 Devhanghae All Rights Reserved.
#include "TextTo3DRequest.h"
#include "Manager/FShapEProcessManager.h"

#define LOCTEXT_NAMESPACE "FTextTo3DRequestModule"


#if WITH_EDITOR

#include "Helper/TextTo3DRequestCommands.h"
#include "UI/SShapEGenerationWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"


static const FName ShapETabId("ShapETab");

TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SShapEGenerationWidget)
        ];
}
#endif


void FTextTo3DRequestModule::StartupModule()
{
    ShapEProcessManager = MakeShared<FShapEProcessManager>();

#if WITH_EDITOR
    FTextTo3DRequestCommands::Register();
    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FTextTo3DRequestCommands::Get().OpenPluginWindow,
        FExecuteAction::CreateRaw(this, &FTextTo3DRequestModule::PluginButtonClicked),
        FCanExecuteAction());

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ShapETabId, FOnSpawnTab::CreateStatic(&OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("ShapETabTitle", "Shap-E Generator"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    RegisterMenus();
#endif
}

void FTextTo3DRequestModule::ShutdownModule()
{
#if WITH_EDITOR
    UToolMenus::UnregisterOwner(this);
    FTextTo3DRequestCommands::Unregister();
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ShapETabId);
#endif

    if (ShapEProcessManager.IsValid())
    {
        ShapEProcessManager->RequestStopProcess();
        ShapEProcessManager.Reset();
    }
}

#if WITH_EDITOR


void FTextTo3DRequestModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(ShapETabId);
}

void FTextTo3DRequestModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    {
        FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
        Section.AddMenuEntryWithCommandList(FTextTo3DRequestCommands::Get().OpenPluginWindow, PluginCommands);
    }
}

#endif 

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTextTo3DRequestModule, TextTo3DRequest)