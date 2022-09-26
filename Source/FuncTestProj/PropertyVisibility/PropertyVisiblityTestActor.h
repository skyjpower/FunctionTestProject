// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "IDetailCustomization.h"

#include "PropertyVisiblityTestActor.generated.h"

struct EVisibility;

/*
	���õ� Value type
*/
UENUM(BlueprintType)
enum class ESelectedValueType : uint8
{
	None,
	Int,
	Float,
	String,
};

UCLASS()
class FUNCTESTPROJ_API APropertyVisiblityTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APropertyVisiblityTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(Category = EditTest, EditAnywhere, meta = (DisplayName = "Key Value"))
		int32 m_IntValue;

	UPROPERTY(Category = EditTest, EditAnywhere, meta = (DisplayName = "Key Value"))
		float m_FloatValue;

	UPROPERTY(Category = EditTest, EditAnywhere, meta = (DisplayName = "Key Value"))
		FString m_StringValue;

	UPROPERTY(Category = EditTest, EditAnywhere, meta = (DisplayName = "Key Query"))
		ESelectedValueType m_SelectedValueType;

	/*
		��� ���� Name � ������ �� �ֵ��� friend ����
	*/
	friend class FPropertyVisiblityTestActorDetails;
};

class FPropertyVisiblityTestActorDetails : public IDetailCustomization
{
public:
	FPropertyVisiblityTestActorDetails();

	/*
		[APropertyVisiblityTestActor] Ŭ������ �������� �� ����� Detail ��ü�� ��ȯ
	*/
	static TSharedRef<IDetailCustomization> MakeInstance();

	/* 
		Property Visibility ���Ǻ� ���� ����� �ۼ��� �Լ�
	*/
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

	/*
		�������� �� �ִ��� ����
	*/
	bool IsEditingEnabled() const;

private:
	/*
		APropertyVisiblityTestActor::m_SelectedValueType�� �� ��ȭ �̺�Ʈ�� ���� �Լ�
	*/
	void OnChangeSelectedValueType();

	/*
		APropertyVisiblityTestActor::m_IntValue ������ ���� ����
	*/
	EVisibility GetIntValueVisibility() const;
	/*
		APropertyVisiblityTestActor::m_FloatValue ������ ���� ����
	*/
	EVisibility GetFloatValueVisibility() const;
	/*
		APropertyVisiblityTestActor::m_StringValue ������ ���� ����
	*/
	EVisibility GetStringValueVisibility() const;

private:
	/*
		�������� ��� ĳ��
	*/
	TWeakObjectPtr<APropertyVisiblityTestActor> m_CachedPropertyVisiblityTestActor;

	/*
		�������� ����� ���� APropertyVisiblityTestActor::m_SelectedValueType ĳ��
		: �� Ÿ�Կ� ���� � [ Key Value ] �� ������ �� �����մϴ�.
	*/
	ESelectedValueType m_CachedSelectedValueType;
};