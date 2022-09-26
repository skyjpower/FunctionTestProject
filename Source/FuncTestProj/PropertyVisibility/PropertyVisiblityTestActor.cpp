// Fill out your copyright notice in the Description page of Project Settings.


#include "PropertyVisiblityTestActor.h"

/*
	Ŀ���� ������ �г� ����� �����Ű�� ���� ���
*/
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "Layout/Visibility.h"

/*
	Ŀ���� ������ �г��� ������ֱ� ���� ���
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
		[����] �� �ڵ���� ���� ����� StartupModule() �ʿ��� �������ִ°� �����ϳ�, �׽�Ʈ ���� �� �� ���� �ۼ��մϴ�.
		: APropertyVisiblityTestActor Ŭ������ CustomClassLayout�� ���� ���� FPropertyVisiblityTestActorDetails�� ���
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
	// >> : ������ ��� ã�� ĳ���ϱ�
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
	// << : ������ ��� ã�� ĳ���ϱ�

	// Property�� IsEnabled ���� ����. IsEditingEnabled()�� false�� ������ ���, ���������� ���ϵ��� ȸ������ Block ó�� �ȴ�.
	TAttribute<bool> PropertyEditCheck = TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::IsEditingEnabled));

	// ����� Property�� �����ִ� Category ��������
	IDetailCategoryBuilder& EditTestCategory = InDetailLayout.EditCategory("EditTest");

	// ������ �� Enum ������ Change Event ���ε��ϱ�
	TSharedPtr<IPropertyHandle> CondPropertyHandle = InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_SelectedValueType), APropertyVisiblityTestActor::StaticClass());
	if (CondPropertyHandle.IsValid() == true)
	{
		FSimpleDelegate OnKeyChangedDelegate = FSimpleDelegate::CreateSP(this, &FPropertyVisiblityTestActorDetails::OnChangeSelectedValueType);
		CondPropertyHandle->SetOnPropertyValueChanged(OnKeyChangedDelegate);
		OnChangeSelectedValueType();
	}

	// APropertyVisiblityTestActor::m_IntValue�� ���� ���� ����
	IDetailPropertyRow& IntValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_IntValue)));
	IntValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetIntValueVisibility)));
	IntValueRow.IsEnabled(PropertyEditCheck);

	// APropertyVisiblityTestActor::m_FloatValue�� ���� ���� ����
	IDetailPropertyRow& FloatValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_FloatValue)));
	FloatValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetFloatValueVisibility)));
	FloatValueRow.IsEnabled(PropertyEditCheck);

	// APropertyVisiblityTestActor::m_StringValue�� ���� ���� ����
	IDetailPropertyRow& StringValueRow = EditTestCategory.AddProperty(InDetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(APropertyVisiblityTestActor, m_StringValue)));
	StringValueRow.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FPropertyVisiblityTestActorDetails::GetStringValueVisibility)));
	StringValueRow.IsEnabled(PropertyEditCheck);
}

bool FPropertyVisiblityTestActorDetails::IsEditingEnabled() const
{
	/*
		���� ���� �������� ����
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

	// ������ ����� ����� SelectedValueType ����
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