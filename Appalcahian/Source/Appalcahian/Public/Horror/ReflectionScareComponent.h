#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReflectionScareComponent.generated.h"

class UMeshComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReflectionScare);

/**
 * The post-server-room aftermath: reflective surfaces occasionally show the white eyes for a frame.
 *
 * Setup: put this on any window / puddle / metal panel actor. The surface material needs a scalar
 * parameter (default "EyesOpacity", 0 by default) that fades in an eyes texture over the reflection.
 * Each new "look" at the surface rolls Chance once; a shared cooldown stops surfaces chaining.
 */
UCLASS(ClassGroup = (Horror), meta = (BlueprintSpawnableComponent))
class APPALCAHIAN_API UReflectionScareComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReflectionScareComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fire now, ignoring chance and cooldown. Use for scripted beats (valve station 3). */
	UFUNCTION(BlueprintCallable, Category = "Reflection Scare")
	void ForceScare();

	/** Override which mesh gets the material parameter. Defaults to the owner's first mesh. */
	UFUNCTION(BlueprintCallable, Category = "Reflection Scare")
	void SetTargetMesh(UMeshComponent* Mesh, int32 MaterialIndex = 0);

	/** Chance per look, 0-1. Design doc says ~1 in 20. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare", meta = (ClampMin = "0", ClampMax = "1"))
	float Chance = 0.05f;

	/** Seconds after any reflection scare (from any surface) before another may fire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare", meta = (ClampMin = "0"))
	float SharedCooldown = 90.f;

	/** Only armed once this story flag is set. Leave None to always be armed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare")
	FName RequiredFlag = TEXT("ServerRoomScareDone");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare", meta = (ClampMin = "50"))
	float MaxLookDistance = 800.f;

	/** How directly the player must be looking at the surface, degrees off center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare", meta = (ClampMin = "1", ClampMax = "60"))
	float LookConeHalfAngle = 15.f;

	/** Rendered frames the eyes stay up. 1 is the design; 2 is safer at very high framerates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare", meta = (ClampMin = "1"))
	int32 FlashFrames = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflection Scare")
	FName EyesParameterName = TEXT("EyesOpacity");

	UPROPERTY(BlueprintAssignable, Category = "Reflection Scare")
	FOnReflectionScare OnScare;

protected:
	virtual void BeginPlay() override;

private:
	bool IsPlayerLookingAtOwner() const;
	void SetEyes(float Value);

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SurfaceMaterial;

	bool bWasLooking = false;
	int32 FramesRemaining = 0;
};
