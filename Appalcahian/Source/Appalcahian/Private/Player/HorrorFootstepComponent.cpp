#include "Player/HorrorFootstepComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UHorrorFootstepComponent::UHorrorFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHorrorFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}

	const float Speed = Movement->Velocity.Size2D();
	const bool bMoving = Movement->IsMovingOnGround() && Speed > MovingSpeedThreshold;

	if (bMoving)
	{
		if (!bWasMoving)
		{
			// First step lands quickly after starting off.
			DistanceSinceStep = StepDistance * 0.6f;
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(EchoTimer);
			}
		}

		DistanceSinceStep += Speed * DeltaTime;
		if (DistanceSinceStep >= StepDistance)
		{
			DistanceSinceStep -= StepDistance;
			PlayStep();
		}
	}
	else if (bWasMoving)
	{
		if (bEchoEnabled && StepsSinceStop > 0)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(EchoTimer, this, &UHorrorFootstepComponent::PlayStopEcho, EchoDelay, false);
			}
		}
		StepsSinceStop = 0;
	}

	bWasMoving = bMoving;
}

USoundBase* UHorrorFootstepComponent::FindStepSound() const
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return DefaultStepSound;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepTrace), false, Owner);
	Params.bReturnPhysicalMaterial = true;

	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, 250.f);
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		const EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Hit);
		if (const TObjectPtr<USoundBase>* Found = SurfaceStepSounds.Find(Surface))
		{
			if (*Found)
			{
				return *Found;
			}
		}
	}

	return DefaultStepSound;
}

void UHorrorFootstepComponent::PlayStep()
{
	USoundBase* Sound = FindStepSound();
	const AActor* Owner = GetOwner();
	if (!Sound || !Owner)
	{
		return;
	}

	LastStepSound = Sound;
	++StepsSinceStop;

	const FVector Feet = Owner->GetActorLocation();
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Feet);
	OnFootstep.Broadcast(false);

	if (bEchoEnabled && DoublingVolume > 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			FTimerHandle Unused;
			const float Volume = DoublingVolume;
			World->GetTimerManager().SetTimer(Unused, FTimerDelegate::CreateWeakLambda(this, [this, Sound, Volume]()
			{
				if (const AActor* CurrentOwner = GetOwner())
				{
					UGameplayStatics::PlaySoundAtLocation(this, Sound, CurrentOwner->GetActorLocation(), Volume);
				}
			}), DoublingDelay, false);
		}
	}
}

void UHorrorFootstepComponent::PlayStopEcho()
{
	const AActor* Owner = GetOwner();
	USoundBase* Sound = EchoSound ? EchoSound.Get() : LastStepSound.Get();
	if (!Owner || !Sound)
	{
		return;
	}

	const FVector Behind = Owner->GetActorLocation() - Owner->GetActorForwardVector() * EchoDistanceBehind;
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Behind, EchoVolume);

	++StopEchoCount;
	OnFootstep.Broadcast(true);
}

void UHorrorFootstepComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	Super::EndPlay(EndPlayReason);
}
