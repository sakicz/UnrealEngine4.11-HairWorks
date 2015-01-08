// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_BlackboardBase.generated.h"

class UBlackboardComponent;

UCLASS(Abstract)
class AIMODULE_API UBTDecorator_BlackboardBase : public UBTDecorator
{
	GENERATED_UCLASS_BODY()

	/** initialize any asset related data */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	/** notify about change in blackboard keys */
	virtual void OnBlackboardChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif

	/** get name of selected blackboard key */
	FName GetSelectedBlackboardKey() const;

protected:

	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category=Blackboard)
	FBlackboardKeySelector BlackboardKey;

	FOnBlackboardChange BBKeyObserver;

	TMap<FBlackboard::FKey, FDelegateHandle> BBKeyObserverDelegateHandles;

	virtual void PostInitProperties() override;

	/** called when execution flow controller becomes active */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** called when execution flow controller becomes inactive */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

FORCEINLINE FName UBTDecorator_BlackboardBase::GetSelectedBlackboardKey() const
{
	return BlackboardKey.SelectedKeyName;
}
