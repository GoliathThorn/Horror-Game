#include "Player/HorrorFlashlightComponent.h"
#include "Player/FlashlightProfile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UHorrorFlashlightComponent::FBeamState UHorrorFlashlightComponent::FBeamState::FromProfile(const UFlashlightProfile* Profile)
{
	FBeamState State;
	if (Profile)
	{
		State.ConeAngle = Profile->ConeAngle;
		State.HotspotRatio = Profile->HotspotRatio;
		State.Intensity = Profile->Intensity;
		State.Temperature = Profile->TemperatureKelvin;
		State.Radius = Profile->AttenuationRadius;
		State.Scattering = Profile->VolumetricScattering;
		State.FlickerStrength = Profile->FlickerStrength;
		State.FlickerSpeed = Profile->FlickerSpeed;
	}
	return State;
}

UHorrorFlashlightComponent::FBeamState UHorrorFlashlightComponent::FBeamState::Lerp(const FBeamState& A, const FBeamState& B, float Alpha)
{
	FBeamState Out;
	Out.ConeAngle = FMath::Lerp(A.ConeAngle, B.ConeAngle, Alpha);
	Out.HotspotRatio = FMath::Lerp(A.HotspotRatio, B.HotspotRatio, Alpha);
	Out.Intensity = FMath::Lerp(A.Intensity, B.Intensity, Alpha);
	Out.Temperature = FMath::Lerp(A.Temperature, B.Temperature, Alpha);
	Out.Radius = FMath::Lerp(A.Radius, B.Radius, Alpha);
	Out.Scattering = FMath::Lerp(A.Scattering, B.Scattering, Alpha);
	Out.FlickerStrength = FMath::Lerp(A.FlickerStrength, B.FlickerStrength, Alpha);
	Out.FlickerSpeed = FMath::Lerp(A.FlickerSpeed, B.FlickerSpeed, Alpha);
	return Out;
}

UHorrorFlashlightComponent::UHorrorFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Setters below (cone, temperature, radius) only work at runtime on movable lights.
	Mobility = EComponentMobility::Movable;
	IntensityUnits = ELightUnits::Candelas;
	bUseTemperature = true;
	CastShadows = true;
}

void UHorrorFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentBeam = FBeamState::FromProfile(StartingProfile);
	bIsOn = bStartOn;
	SetVisibility(bIsOn);
	PushBeamToLight(CurrentBeam, 1.f);
}

void UHorrorFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBlending)
	{
		BlendElapsed += DeltaTime;
		const float Alpha = BlendDuration > 0.f ? FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f) : 1.f;
		CurrentBeam = FBeamState::Lerp(BlendFrom, BlendTo, Alpha);
		bBlending = Alpha < 1.f;
	}

	if (!bIsOn || bIsDead)
	{
		return;
	}

	float FlickerMultiplier = 1.f;
	if (CurrentBeam.FlickerStrength > KINDA_SMALL_NUMBER)
	{
		FlickerTime += DeltaTime * CurrentBeam.FlickerSpeed;
		// Perlin gives a smooth wander; cubing it keeps the light mostly steady with occasional deeper dips.
		const float Noise = (FMath::PerlinNoise1D(FlickerTime) + 1.f) * 0.5f;
		FlickerMultiplier = 1.f - CurrentBeam.FlickerStrength * Noise * Noise * Noise;
	}

	PushBeamToLight(CurrentBeam, FlickerMultiplier);
}

void UHorrorFlashlightComponent::ApplyProfile(UFlashlightProfile* Profile, float BlendTime)
{
	if (!Profile)
	{
		return;
	}

	BlendTo = FBeamState::FromProfile(Profile);
	if (BlendTime <= 0.f)
	{
		CurrentBeam = BlendTo;
		bBlending = false;
		PushBeamToLight(CurrentBeam, 1.f);
		return;
	}

	BlendFrom = CurrentBeam;
	BlendDuration = BlendTime;
	BlendElapsed = 0.f;
	bBlending = true;
}

void UHorrorFlashlightComponent::SetFlashlightOn(bool bOn)
{
	if (bIsDead)
	{
		PlayUISound(DeadClickSound);
		return;
	}

	if (bIsOn == bOn)
	{
		return;
	}

	bIsOn = bOn;
	SetVisibility(bIsOn);
	PlayUISound(bIsOn ? ClickOnSound : ClickOffSound);
	OnFlashlightToggled.Broadcast(bIsOn);
}

void UHorrorFlashlightComponent::ToggleFlashlight()
{
	SetFlashlightOn(!bIsOn);
}

void UHorrorFlashlightComponent::KillFlashlight()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsOn = false;
	SetVisibility(false);
	PlayUISound(DeadClickSound);
	OnFlashlightDied.Broadcast();
}

void UHorrorFlashlightComponent::ReplaceFlashlight(UFlashlightProfile* NewProfile)
{
	bIsDead = false;
	ApplyProfile(NewProfile, 0.f);
	SetFlashlightOn(true);
}

void UHorrorFlashlightComponent::PushBeamToLight(const FBeamState& State, float FlickerMultiplier)
{
	const float HalfAngle = State.ConeAngle * 0.5f;
	SetOuterConeAngle(HalfAngle);
	SetInnerConeAngle(HalfAngle * State.HotspotRatio);
	SetIntensity(State.Intensity * FlickerMultiplier);
	SetTemperature(State.Temperature);
	SetAttenuationRadius(State.Radius);
	SetVolumetricScatteringIntensity(State.Scattering);
}

void UHorrorFlashlightComponent::PlayUISound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
