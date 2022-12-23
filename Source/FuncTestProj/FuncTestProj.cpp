// Copyright Epic Games, Inc. All Rights Reserved.

#include "FuncTestProj.h"

#include "Modules/ModuleManager.h"
 
#include "PropertyEditorModule.h"
#include "ISettingsModule.h"

#include "FuncTestProj/PathFollowing/MyNavigationSystem.h"
#include "FuncTestProj/PropertyVisibility/PropertyVisiblityTestActor.h"

#define LOCTEXT_NAMESPACE "FFuncTestProjModule"

class FFuncTestProj : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogClass, Log, TEXT("FFuncTestProj::StartupModule()"));

		/*
			APropertyVisiblityTestActor 클래스의 CustomClassLayout을 새로 만든 FPropertyVisiblityTestActorDetails로 등록
		*/
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout("PropertyVisiblityTestActor", FOnGetDetailCustomizationInstance::CreateStatic(&FPropertyVisiblityTestActorDetails::MakeInstance));

		/*
			커스텀 프로젝트 세팅 카테고리 생성
		*/
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		if (SettingsModule != nullptr)
		{
			UMyNavigationSystem* NavSysCDO = UMyNavigationSystem::StaticClass()->GetDefaultObject<UMyNavigationSystem>();
			if (NavSysCDO != nullptr)
			{
				SettingsModule->RegisterSettings("Project", "Engine", "MyNavigationSystem",
					LOCTEXT("MyNavigationSystemSettingsName", "My Navigation System"),
					LOCTEXT("MyNavigationSystemSettingsDescription", "Settings for the navigation system."),
					NavSysCDO
				);
			}
		}
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogClass, Log, TEXT("FFuncTestProj::ShutdownModule()"));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFuncTestProj, FuncTestProj, "FuncTestProj");

#undef LOCTEXT_NAMESPACE