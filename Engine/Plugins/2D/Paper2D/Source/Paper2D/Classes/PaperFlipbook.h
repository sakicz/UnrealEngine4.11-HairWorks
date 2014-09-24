// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PaperSprite.h"
#include "PaperFlipbook.generated.h"

USTRUCT()
struct FPaperFlipbookKeyFrame
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category=Sprite, EditAnywhere)
	UPaperSprite* Sprite;

	UPROPERTY(Category=Sprite, EditAnywhere, meta=(ClampMin=1))
	int32 FrameRun;

	FPaperFlipbookKeyFrame()
		: Sprite(NULL)
		, FrameRun(1)
	{
	}
};

/**
 * Contains an animation sequence of sprite frames
 */
UCLASS(BlueprintType, meta = (DisplayThumbnail = "true"))
class PAPER2D_API UPaperFlipbook : public UObject
{
	GENERATED_UCLASS_BODY()

protected:
	// The nominal frame rate to play this flipbook animation back at
	UPROPERTY(Category=Sprite, EditAnywhere, BlueprintReadOnly)
	float FramesPerSecond;

	// The set of key frames for this flipbook animation (each one has a duration and a sprite to display)
	UPROPERTY(Category=Sprite, EditAnywhere)
	TArray<FPaperFlipbookKeyFrame> KeyFrames;

	// The material to use on a flipbook player instance if not overridden
	UPROPERTY(Category=Sprite, EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* DefaultMaterial;

public:
	// Returns the nominal frame rate to play this flipbook animation back at
	float GetFramesPerSecond() const;

	// Returns the total number of frames
	UFUNCTION(BlueprintCallable, Category="Sprite")
	int32 GetNumFrames() const;

	// Returns the total duration in seconds
	UFUNCTION(BlueprintCallable, Category="Sprite")
	float GetTotalDuration() const;

	// Returns the keyframe index that covers the specified time (in seconds), or INDEX_NONE if none exists.
	// When bClampToEnds is true, it will choose the first or last keyframe if the time is out of range.
	UFUNCTION(BlueprintCallable, Category="Sprite")
	int32 GetKeyFrameIndexAtTime(float Time, bool bClampToEnds = false) const;

	// Returns the sprite at the specified time (in seconds), or nullptr if none exists.
	// When bClampToEnds is true, it will choose the first or last sprite if the time is out of range.
	UFUNCTION(BlueprintCallable, Category="Sprite")
	UPaperSprite* GetSpriteAtTime(float Time, bool bClampToEnds = false) const;

	// Returns the sprite at the specified keyframe index, or nullptr if none exists
	UFUNCTION(BlueprintCallable, Category="Sprite")
	UPaperSprite* GetSpriteAtFrame(int32 FrameIndex) const;

	// Returns the render bounds of this sprite
	FBoxSphereBounds GetRenderBounds() const;

	// Returns the number of key frames
	UFUNCTION(BlueprintCallable, Category="Sprite")
	int32 GetNumKeyFrames() const
#if CPP
	{
		return KeyFrames.Num();
	}
#endif

	// Is the specified Index within the valid range of key frames?
	UFUNCTION(BlueprintCallable, Category="Sprite")
	bool IsValidKeyFrameIndex(int32 Index) const
#if CPP
	{
		return KeyFrames.IsValidIndex(Index);
	}
#endif

	// Returns the key frame at the specified index, make sure the index is valid before use
	const FPaperFlipbookKeyFrame& GetKeyFrameChecked(int32 Index) const
	{
		return KeyFrames[Index];
	}

	// Search for a socket at the specified frame
	bool FindSocket(FName SocketName, int32 KeyFrameIndex, FTransform& OutLocalTransform);

	// Returns true if the flipbook has any sockets
	bool HasAnySockets() const;

	// Returns a list of all of the sockets
	void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const;

	// Return the default material for this flipbook
	UMaterialInterface* GetDefaultMaterial() const { return DefaultMaterial; }

	// Rebuilds cached data about the animation (such as total number of frames that the keyframes span, etc...)
	void InvalidateCachedData();

	// UObject interface
#if WITH_EDITOR
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

private:
	friend class FScopedFlipbookMutator;
};


// Helper class to edit properties of a UPaperFlipbook while ensuring that cached data remains up to date
class FScopedFlipbookMutator
{
public:
	float& FramesPerSecond;
	TArray<FPaperFlipbookKeyFrame>& KeyFrames;

private:
	UPaperFlipbook* SourceFlipbook;

public:
	FScopedFlipbookMutator(UPaperFlipbook* InFlipbook)
		: FramesPerSecond(InFlipbook->FramesPerSecond)
		, KeyFrames(InFlipbook->KeyFrames)
		, SourceFlipbook(InFlipbook)
	{
	}

	~FScopedFlipbookMutator()
	{
		InvalidateCachedData();
	}

	void InvalidateCachedData()
	{
		SourceFlipbook->InvalidateCachedData();
	}

	UPaperFlipbook* GetSourceFlipbook() const
	{
		return SourceFlipbook;
	}
};
