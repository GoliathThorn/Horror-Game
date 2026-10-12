#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HorrorFootstepComponent.generated.h"

class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFootstep, bool, bIsEcho);

/**
 * Distance-based footsteps with per-surface sounds, plus the Act 5.4 echo.
 *
 * With bEchoEnabled:
 *  - every step gets a quiet doubled copy a beat later (reads as corridor reverb), and
 *  - every time the player stops, one more step lands EchoDelay seconds later, behind them.
 * It is deliberately identical every time so the player can test it and get the same result.
 * Use a single Sound Wave (not a randomizing Cue) for EchoSound for that reason.
 */
UCLASS(ClassGroup = (Horror), meta = (BlueprintSpawnableComponent))
class APPALCAHIAN_API UHorrorFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHorrorFootstepComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Footsteps")
	void SetEchoEnabled(bool bEnabled) { bEchoEnabled = bEnabled; }

	/** How many times the stop-echo has fired since enabled. Lets a Blueprint wait for the second test. */
	UFUNCTION(BlueprintPure, Category = "Footsteps")
	int32 GetStopEchoCount() const { return StopEchoCount; }

	/** cm travelled per step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps", meta = (ClampMin = "20"))
	float StepDistance = 140.f;

	/** Below this speed the player counts as stopped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps", meta = (ClampMin = "0"))
	float MovingSpeedThreshold = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
	TObjectPtr<USoundBase> DefaultStepSound;

	/** Physical surface -> step sound. Set surfaces up in Project Settings > Physics > Physical Surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> SurfaceStepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo")
	bool bEchoEnabled = false;

	/** The extra step. Leave empty to reuse the last surface sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo")
	TObjectPtr<USoundBase> EchoSound;

	/** Seconds after stopping before the extra step lands. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo", meta = (ClampMin = "0.05"))
	float EchoDelay = 0.45f;

	/** How far behind the player the extra step sounds, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo", meta = (ClampMin = "0"))
	float EchoDistanceBehind = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo", meta = (ClampMin = "0"))
	float EchoVolume = 0.9f;

	/** The "slightly reinforced" doubling on every step while echo is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo", meta = (ClampMin = "0"))
	float DoublingDelay = 0.09f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Echo", meta = (ClampMin = "0"))
	float DoublingVolume = 0.35f;

	UPROPERTY(BlueprintAssignable, Category = "Footsteps")
	FOnFootstep OnFootstep;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void PlayStep();
	void PlayStopEcho();
	USoundBase* FindStepSound() const;

	UPROPERTY()
	TObjectPtr<USoundBase> LastStepSound;

	FTimerHandle EchoTimer;
	float DistanceSinceStep = 0.f;
	int32 StepsSinceStop = 0;
	int32 StopEchoCount = 0;
	bool bWasMoving = false;
};
