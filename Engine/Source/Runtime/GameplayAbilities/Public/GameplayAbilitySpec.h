// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayPrediction.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.generated.h"

/**
 *	This file exists in addition so that GameplayEffect.h can use FGameplayAbilitySpec without having to include GameplayAbilityTypes.h which has depancies on
 *	GameplayEffect.h
 */

USTRUCT(BlueprintType)
struct FGameplayAbilitySpecHandle
{
	GENERATED_USTRUCT_BODY()

	FGameplayAbilitySpecHandle()
	: Handle(INDEX_NONE)
	{

	}

	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}

	void GenerateNewHandle()
	{
		static int32 GHandle = 1;
		Handle = GHandle++;
	}

	bool operator==(const FGameplayAbilitySpecHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FGameplayAbilitySpecHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	friend uint32 GetTypeHash(const FGameplayAbilitySpecHandle& SpecHandle)
	{
		return ::GetTypeHash(SpecHandle.Handle);
	}

private:

	UPROPERTY()
	int32 Handle;
};

UENUM(BlueprintType)
namespace EGameplayAbilityActivationMode
{
	enum Type
	{
		// We are the authority activating this ability
		Authority,

		// We are not the authority but aren't predicting yet. This is a mostly invalid state to be in.
		NonAuthority,

		// We are predicting the activation of this ability
		Predicting,

		// We are not the authority, but the authority has confirmed this activation
		Confirmed,
	};
}

/** Describes what happens when a GameplayEffect, that is granting an active ability, is removed from its owner. */
UENUM()
enum class EGameplayEffectGrantedAbilityRemovePolicy : uint8
{
	/** Active abilities are immediately canceled and the ability is removed. */
	CancelAbilityImmediately,
	/** Active abilities are allowed to finish, and then removed. */
	RemoveAbilityOnEnd,
	/** Granted abilties are left lone when the granting GameplayEffect is removed. */
	DoNothing,
};

/** This is data that can be used to create an FGameplayAbilitySpec. Has some data that is only relevant when granted by a GameplayEffect */
USTRUCT(BlueprintType)
struct FGameplayAbilitySpecDef
{
	FGameplayAbilitySpecDef()
		: Level(INDEX_NONE)
		, InputID(INDEX_NONE)
		, RemovalPolicy(EGameplayEffectGrantedAbilityRemovePolicy::CancelAbilityImmediately)
	{
		LevelScalableFloat.SetValue(1.f);
	}

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Ability Definition", NotReplicated)
	TSubclassOf<UGameplayAbility> Ability;
	
	// Deprecated for LevelScalableFloat
	UPROPERTY(NotReplicated)
	int32 Level; 

	UPROPERTY(EditDefaultsOnly, Category="Ability Definition", NotReplicated, DisplayName="Level")
	FScalableFloat LevelScalableFloat; 
	
	UPROPERTY(EditDefaultsOnly, Category="Ability Definition", NotReplicated)
	int32 InputID;

	UPROPERTY(EditDefaultsOnly, Category="Ability Definition", NotReplicated)
	EGameplayEffectGrantedAbilityRemovePolicy RemovalPolicy;
	
	UPROPERTY(NotReplicated)
	UObject* SourceObject;

	/** This handle can be set if the SpecDef is used to create a real FGameplaybilitySpec */
	UPROPERTY()
	FGameplayAbilitySpecHandle	AssignedHandle;
};

/**
 *	FGameplayAbilityActivationInfo
 *
 *	Data tied to a specific activation of an ability.
 *		-Tell us whether we are the authority, if we are predicting, confirmed, etc.
 *		-Holds current and previous PredictionKey
 *		-Generally not meant to be subclassed in projects.
 *		-Passed around by value since the struct is small.
 */
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FGameplayAbilityActivationInfo
{
	GENERATED_USTRUCT_BODY()

	FGameplayAbilityActivationInfo()
		: ActivationMode(EGameplayAbilityActivationMode::Authority)
		, bCanBeEndedByOtherInstance(false)
	{

	}

	FGameplayAbilityActivationInfo(AActor* InActor)
		: bCanBeEndedByOtherInstance(false)	
	{
		// On Init, we are either Authority or NonAuthority. We haven't been given a PredictionKey and we haven't been confirmed.
		// NonAuthority essentially means 'I'm not sure what how I'm going to do this yet'.
		ActivationMode = (InActor->Role == ROLE_Authority ? EGameplayAbilityActivationMode::Authority : EGameplayAbilityActivationMode::NonAuthority);
	}

	FGameplayAbilityActivationInfo(EGameplayAbilityActivationMode::Type InType)
		: ActivationMode(InType)
		, bCanBeEndedByOtherInstance(false)
	{
	}	

	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	mutable TEnumAsByte<EGameplayAbilityActivationMode::Type>	ActivationMode;

	/** An ability that runs on multiple game instances can be canceled by a remote instance, but only if that remote instance has already confirmed starting it. */
	UPROPERTY()
	bool bCanBeEndedByOtherInstance;

	void SetActivationConfirmed();

	/** Called on client to set this as a predicted ability */
	void SetPredicting(FPredictionKey PredictionKey);

	/** Called on the server to set the key used by the client to predict this ability */
	void ServerSetActivationPredictionKey(FPredictionKey PredictionKey);

	FPredictionKey GetActivationPredictionKey() { return PredictionKeyWhenActivated; }

private:

	// This was the prediction key used to activate this ability. It does not get updated
	// if new prediction keys are generated over the course of the ability.
	UPROPERTY()
	FPredictionKey PredictionKeyWhenActivated;
	
};


/** An activatable ability spec, hosted on the ability system component. This defines both what the ability is (what class, what level, input binding etc)
 *  and also holds runtime state that must be kept outside of the ability being instanced/activated.
 */
USTRUCT()
struct GAMEPLAYABILITIES_API FGameplayAbilitySpec : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FGameplayAbilitySpec()
	: Ability(nullptr), Level(1), InputID(INDEX_NONE), SourceObject(nullptr), InputPressed(false), ActiveCount(0), RemoveAfterActivation(false), PendingRemove(false)
	{
		
	}

	FGameplayAbilitySpec(UGameplayAbility* InAbility, int32 InLevel=1, int32 InInputID=INDEX_NONE, UObject* InSourceObject=nullptr)
		: Ability(InAbility), Level(InLevel), InputID(InInputID), SourceObject(InSourceObject), InputPressed(false), ActiveCount(0), RemoveAfterActivation(false),  PendingRemove(false)
	{
		Handle.GenerateNewHandle();
	}
	
	FGameplayAbilitySpec(FGameplayAbilitySpecDef& InDef, int32 InGameplayEffectLevel, FActiveGameplayEffectHandle InGameplayEffectHandle = FActiveGameplayEffectHandle());

	/** Handle for outside sources to refer to this spec by */
	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
	
	/** Ability of the spec (Always the CDO. This should be const but too many things modify it currently) */
	UPROPERTY()
	UGameplayAbility* Ability;
	
	/** Level of Ability */
	UPROPERTY()
	int32	Level;

	/** InputID, if bound */
	UPROPERTY()
	int32	InputID;

	/** Object this ability was created from, can be an actor or static object. Useful to bind an ability to a gameplay object */
	UPROPERTY()
	UObject* SourceObject;

	/** Is input currently pressed. Set to false when input is released */
	UPROPERTY(NotReplicated)
	bool	InputPressed;

	/** A count of the number of times this ability has been activated minus the number of times it has been ended. For instanced abilities this will be the number of currently active instances. Can't replicate until prediction accurately handles this.*/
	UPROPERTY(NotReplicated)
	uint8	ActiveCount;

	/** If true, this ability should be removed as soon as it finishes executing */
	UPROPERTY(NotReplicated)
	bool	RemoveAfterActivation;

	/** Activation state of this ability. This is not replicated since it needs to be overwritten locally on clients during prediction. */
	UPROPERTY(NotReplicated)
	FGameplayAbilityActivationInfo	ActivationInfo;

	/** Non replicating instances of this ability. */
	UPROPERTY(NotReplicated)
	TArray<UGameplayAbility*> NonReplicatedInstances;

	/** Replicated instances of this ability.. */
	UPROPERTY()
	TArray<UGameplayAbility*> ReplicatedInstances;

	/** Handle to GE that granted us (usually invalid) */
	UPROPERTY(NotReplicated)
	FActiveGameplayEffectHandle	GameplayEffectHandle;

	/** Pending removal due to scope lock */
	UPROPERTY(NotReplicated)
	bool	PendingRemove;

	/** Returns the primary instance, used for instance once abilities */
	UGameplayAbility* GetPrimaryInstance() const;

	/** Returns all instances, which can include instance per execution abilities */
	TArray<UGameplayAbility*> GetAbilityInstances() const
	{
		TArray<UGameplayAbility*> Abilities;
		Abilities.Append(ReplicatedInstances);
		Abilities.Append(NonReplicatedInstances);
		return Abilities;
	}

	/** Returns true if this ability is active in any way */
	bool IsActive() const;

	void PreReplicatedRemove(const struct FGameplayAbilitySpecContainer& InArraySerializer);
	void PostReplicatedAdd(const struct FGameplayAbilitySpecContainer& InArraySerializer);
};


/** Fast serializer wrapper for above struct */
USTRUCT()
struct GAMEPLAYABILITIES_API FGameplayAbilitySpecContainer : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	/** List of activatable abilities */
	UPROPERTY()
	TArray<FGameplayAbilitySpec> Items;

	/** Component that owns this list */
	UAbilitySystemComponent* Owner;

	void RegisterWithOwner(UAbilitySystemComponent* Owner);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayAbilitySpec, FGameplayAbilitySpecContainer>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits< FGameplayAbilitySpecContainer > : public TStructOpsTypeTraitsBase
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

// ----------------------------------------------------

struct GAMEPLAYABILITIES_API FScopedAbilityListLock
{
	FScopedAbilityListLock(UAbilitySystemComponent& InContainer);
	~FScopedAbilityListLock();

private:
	UAbilitySystemComponent& AbilitySystemComponent;
};

#define ABILITYLIST_SCOPE_LOCK()	FScopedAbilityListLock ActiveScopeLock(*this);