#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HorrorControlLockComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnControlLockReleased);

/**
 * The server-room "paralysis": movement input is ignored, look input is scaled way down
 * so the mouse only drifts. Put it on the player character and multiply your look input
 * (IA_Look / IA_MouseLook) by GetLookScale() in the character Blueprint.
 */
UCLASS(ClassGroup = (Horror), meta = (BlueprintSpawnableComponent))
class APPALCAHIAN_API UHorrorControlLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Duration <= 0 locks until ReleaseLock() is called. */
	UFUNCTION(BlueprintCallable, Category = "Horror|Control")
	void LockMovement(float Duration = 1.5f, float LookScaleWhileLocked = 0.08f);

	UFUNCTION(BlueprintCallable, Category = "Horror|Control")
	void ReleaseLock();

	UFUNCTION(BlueprintPure, Category = "Horror|Control")
	bool IsLocked() const { return bLocked; }

	/** 1 normally, reduced while locked. */
	UFUNCTION(BlueprintPure, Category = "Horror|Control")
	float GetLookScale() const { return bLocked ? LockedLookScale : 1.f; }

	UPROPERTY(BlueprintAssignable, Category = "Horror|Control")
	FOnControlLockReleased OnLockReleased;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	class AController* GetOwnerController() const;

	FTimerHandle ReleaseTimer;
	float LockedLookScale = 1.f;
	bool bLocked = false;
};
