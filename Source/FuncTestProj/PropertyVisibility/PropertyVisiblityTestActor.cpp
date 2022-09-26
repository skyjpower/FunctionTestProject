// Fill out your copyright notice in the Description page of Project Settings.


#include "PropertyVisiblityTestActor.h"

/*
	커스텀 디테일 패널 기능을 적용시키기 위한 헤더
*/
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "Layout/Visibility.h"

/*
	커스텀 디테일 패널을 등록해주기 위한 헤더
*/
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

// Sets default values
APropertyVisiblityTestActor::APropertyVisiblityTestActor()
	: m_IntValue(0)
	, m_FloatValue(0.0f)
	, m_StringValue(TEXT("Hello"))
	, m_SelectedValueType(ESelectedValueType::None)
{
	PrimaryActorTick.bCanEverTick = true;

	/*
		[주의] 이 코드들이 속한 모듈의 StartupModule() 쪽에서 세팅해주는게 적절하나, 테스트 편의 상 이 곳에 작성합니다.
		: APropertyVisiblityTestActor 클래스의 CustomClassLayout을 새로 만든 FPropertyVisiblityTestActorDetails로 등록
	*/
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("PropertyVisiblityTestActor", FOnGetDetailCustomizationInstance::CreateStatic(&FPropertyVisiblityTestActorDetails::MakeInstance));
}

// Called when the game starts or when spawned
void APropertyVisiblityTestActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APropertyVisiblityTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#pragma region FPropertyVisiblityTestActorDetails
FPropertyVisiblityTestActorDetails::FPropertyVisiblityTestActorDetails()
	: m_CachedPropertyVisiblityTestActor(nullptr)
	, m_CachedSelectedValueType(ESelectedValueType::None)
{

}

TSharedRef<IDetailCustomization> FPropertyVisiblityTestActorDetails::MakeInstance()
{
	return MakeShareable(new FPropertyVisiblityTestActorDetails);
}

void FPropertyVisiblityTestActorDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	// >> : 에디팅 대상 찾아 캐싱하기
	TArray<TWeakObjectPtr<UObject> > MyOuters;
	InDetailLayout.GetObjectsBeingCustomized(MyOuters);

	m_CachedPropertyVisiblityTestActor.Reset();
	for (int32 i = 0; i < MyOuters.Num(); i++)
	{
		APropertyVisiblityTestActor* EditingActor = Cast<APropertyVisiblityTestActor>(MyOuters[i].Get());
		if (EditingActor != nullptr)
		{
			m_CachedPropertyVisiblityTestActor = EditingActor;
			break;
		}
	}

	if (m_CachedPropertyVisiblityTestActor.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Fail to find m_CachedPropertyVisiblityTestActor[APropertyVisiblityTestActor] in InDetailLayout[IDetailLayoutBuilder] outers"));
		return;
	}
	// << : 에디팅 대상 찾아 캐싱하기

	// Property의 IsEnabled 조건 적용. IsEditingEnabled()가 false를 리턴할 경우, 에디팅하지 못하도록 회색으로 Block 처리 된다.
	TAttribute<bool> PropertyEditCheck = TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::IsEditingEnabled));

	// 고려할 Property가 속해있는 Category 가져오기
	IDetailCategoryBuilder& EditTestCategory = InDetailLayout.EditCategory("EditTest");

	// 조건이 될 Enum 변수의 Change Event 바인딩하기
	TSharedPtr<IPropertyHandle> CondPropertyHandle = InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_SelectedValueType), APropertyVisiblityTestActor::StaticClass());
	if (CondPropertyHandle.IsValid() == true)
	{
		FSimpleDelegate OnKeyChangedDelegate = FSimpleDelegate::CreateSP(this, &FPropertyVisiblityTestActorDetails::OnChangeSelectedValueType);
		CondPropertyHandle->SetOnPropertyValueChanged(OnKeyChangedDelegate);
		OnChangeSelectedValueType();
	}

	// APropertyVisiblityTestActor::m_IntValue에 대한 조건 설정
	IDetailPropertyRow& IntValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_IntValue)));
	IntValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetIntValueVisibility)));
	IntValueRow.IsEnabled(PropertyEditCheck);

	// APropertyVisiblityTestActor::m_FloatValue에 대한 조건 설정
	IDetailPropertyRow& FloatValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_FloatValue)));
	FloatValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetFloatValueVisibility)));
	FloatValueRow.IsEnabled(PropertyEditCheck);

	// APropertyVisiblityTestActor::m_StringValue에 대한 조건 설정
	IDetailPropertyRow& StringValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_StringValue)));
	StringValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetStringValueVisibility)));
	StringValueRow.IsEnabled(PropertyEditCheck);
}

bool FPropertyVisiblityTestActorDetails::IsEditingEnabled() const
{
	/*
		따로 조건 지정하지 않음
	*/
	return true;
}

void FPropertyVisiblityTestActorDetails::OnChangeSelectedValueType()
{
	m_CachedSelectedValueType = ESelectedValueType::None;

	if (m_CachedPropertyVisiblityTestActor.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid m_CachedPropertyVisiblityTestActor[APropertyVisiblityTestActor]"));
		return;
	}

	// 에디팅 대상의 변경된 SelectedValueType 저장
	m_CachedSelectedValueType = m_CachedPropertyVisiblityTestActor->m_SelectedValueType;
}

EVisibility FPropertyVisiblityTestActorDetails::GetIntValueVisibility() const
{
	return m_CachedSelectedValueType == ESelectedValueType::Int ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility FPropertyVisiblityTestActorDetails::GetFloatValueVisibility() const
{
	return m_CachedSelectedValueType == ESelectedValueType::Float ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility FPropertyVisiblityTestActorDetails::GetStringValueVisibility() const
{
	return m_CachedSelectedValueType == ESelectedValueType::String ? EVisibility::Visible : EVisibility::Collapsed;
}
#pragma endregion // FPropertyVisiblityTestActorDetails