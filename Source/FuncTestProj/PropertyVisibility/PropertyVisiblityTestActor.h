// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "IDetailCustomization.h"

#include "PropertyVisiblityTestActor.generated.h"

struct EVisibility;

/*
	선택된 Value type
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
		멤버 변수 Name 등에 접근할 수 있도록 friend 선언
	*/
	friend class FPropertyVisiblityTestActorDetails;
};

class FPropertyVisiblityTestActorDetails : public IDetailCustomization
{
public:
	FPropertyVisiblityTestActorDetails();

	/*
		[APropertyVisiblityTestActor] 클래스를 에디팅할 때 사용할 Detail 객체를 반환
	*/
	static TSharedRef<IDetailCustomization> MakeInstance();

	/* 
		Property Visibility 조건부 설정 기능을 작성할 함수
	*/
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

	/*
		에디팅할 수 있는지 조건
	*/
	bool IsEditingEnabled() const;

private:
	/*
		APropertyVisiblityTestActor::m_SelectedValueType의 값 변화 이벤트를 받을 함수
	*/
	void OnChangeSelectedValueType();

	/*
		APropertyVisiblityTestActor::m_IntValue 변수의 노출 조건
	*/
	EVisibility GetIntValueVisibility() const;
	/*
		APropertyVisiblityTestActor::m_FloatValue 변수의 노출 조건
	*/
	EVisibility GetFloatValueVisibility() const;
	/*
		APropertyVisiblityTestActor::m_StringValue 변수의 노출 조건
	*/
	EVisibility GetStringValueVisibility() const;

private:
	/*
		에디팅할 대상 캐싱
	*/
	TWeakObjectPtr<APropertyVisiblityTestActor> m_CachedPropertyVisiblityTestActor;

	/*
		에디팅할 대상의 현재 APropertyVisiblityTestActor::m_SelectedValueType 캐싱
		: 이 타입에 따라 어떤 [ Key Value ] 를 보여줄 지 지정합니다.
	*/
	ESelectedValueType m_CachedSelectedValueType;
};