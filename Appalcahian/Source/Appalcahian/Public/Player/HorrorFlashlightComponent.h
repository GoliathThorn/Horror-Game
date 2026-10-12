#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "HorrorFlashlightComponent.generated.h"

class UFlashlightProfile;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFlashlightDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);

/**
 * Drop this on the player's camera in place of a regular spot light.
 * Assign StartingProfile (God Beam). At the Act 4.1 beat call KillFlashlight();
 * when the D-cell is picked up off the corpse call ReplaceFlashlight(DCellProfile).
 */
UCLASS(ClassGroup = (Horror), meta = (BlueprintSpawnableComponent))
class APPALCAHIAN_API UHorrorFlashlightComponent : public USpotLightComponent
{
	GENERATED_BODY()

public:
	UHorrorFlashlightComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Switch beam feel. BlendTime 0 = instant. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void ApplyProfile(UFlashlightProfile* Profile, float BlendTime = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void SetFlashlightOn(bool bOn);

	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void ToggleFlashlight();

	UFUNCTION(BlueprintPure, Category = "Flashlight")
	bool IsFlashlightOn() const { return bIsOn; }

	/** Dead click. Light goes out and stays out; toggling afterwards only clicks. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void KillFlashlight();

	UFUNCTION(BlueprintPure, Category = "Flashlight")
	bool IsDead() const { return bIsDead; }

	/** New flashlight in hand: clears the dead state, applies the profile, turns on. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void ReplaceFlashlight(UFlashlightProfile* NewProfile);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
	TObjectPtr<UFlashlightProfile> StartingProfile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
	bool bStartOn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Audio")
	TObjectPtr<USoundBase> ClickOnSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Audio")
	TObjectPtr<USoundBase> ClickOffSound;

	/** Played when it dies and on every toggle attempt while dead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Audio")
	TObjectPtr<USoundBase> DeadClickSound;

	UPROPERTY(BlueprintAssignable, Category = "Flashlight")
	FOnFlashlightDied OnFlashlightDied;

	UPROPERTY(BlueprintAssignable, Category = "Flashlight")
	FOnFlashlightToggled OnFlashlightToggled;

private:
	struct FBeamState
	{
		float ConeAngle = 70.f;
		float HotspotRatio = 0.5f;
		float Intensity = 2000.f;
		float Temperature = 6500.f;
		float Radius = 4000.f;
		float Scattering = 1.f;
		float FlickerStrength = 0.f;
		float FlickerSpeed = 8.f;

		static FBeamState FromProfile(const UFlashlightProfile* Profile);
		static FBeamState Lerp(const FBeamState& A, const FBeamState& B, float Alpha);
	};

	void PushBeamToLight(const FBeamState& State, float FlickerMultiplier);
	void PlayUISound(USoundBase* Sound) const;

	FBeamState CurrentBeam;
	FBeamState BlendFrom;
	FBeamState BlendTo;
	float BlendDuration = 0.f;
	float BlendElapsed = 0.f;
	bool bBlending = false;

	float FlickerTime = 0.f;
	bool bIsOn = false;
	bool bIsDead = false;
};
