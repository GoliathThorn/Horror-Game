#include "Player/HorrorControlLockComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

AController* UHorrorControlLockComponent::GetOwnerController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	return Pawn ? Pawn->GetController() : nullptr;
}

void UHorrorControlLockComponent::LockMovement(float Duration, float LookScaleWhileLocked)
{
	LockedLookScale = FMath::Clamp(LookScaleWhileLocked, 0.f, 1.f);

	if (!bLocked)
	{
		bLocked = true;

		if (AController* Controller = GetOwnerController())
		{
			Controller->SetIgnoreMoveInput(true);
		}

		const ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr)
		{
			Movement->StopMovementImmediately();
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReleaseTimer);
		if (Duration > 0.f)
		{
			World->GetTimerManager().SetTimer(ReleaseTimer, this, &UHorrorControlLockComponent::ReleaseLock, Duration, false);
		}
	}
}

void UHorrorControlLockComponent::ReleaseLock()
{
	if (!bLocked)
	{
		return;
	}

	bLocked = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReleaseTimer);
	}

	if (AController* Controller = GetOwnerController())
	{
		Controller->SetIgnoreMoveInput(false);
	}

	OnLockReleased.Broadcast();
}

void UHorrorControlLockComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseLock();
	Super::EndPlay(EndPlayReason);
}
