#include "Core/HorrorGameSubsystem.h"
#include "Appalcahian.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

UHorrorGameSubsystem* UHorrorGameSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UHorrorGameSubsystem>() : nullptr;
}

void UHorrorGameSubsystem::SetFlag(FName Flag, bool bValue)
{
	if (Flag.IsNone())
	{
		return;
	}

	const bool bHadFlag = Flags.Contains(Flag);
	if (bHadFlag == bValue)
	{
		return;
	}

	if (bValue)
	{
		Flags.Add(Flag);
	}
	else
	{
		Flags.Remove(Flag);
	}

	UE_LOG(LogAppalcahian, Log, TEXT("Flag %s -> %s"), *Flag.ToString(), bValue ? TEXT("set") : TEXT("cleared"));
	OnFlagChanged.Broadcast(Flag, bValue);
}

bool UHorrorGameSubsystem::HasFlag(FName Flag) const
{
	return Flags.Contains(Flag);
}

void UHorrorGameSubsystem::ChooseEnding(EHorrorEnding NewEnding)
{
	// First choice wins; the player can't change their mind once an ending is committed.
	if (Ending != EHorrorEnding::None || NewEnding == EHorrorEnding::None)
	{
		return;
	}

	Ending = NewEnding;
	OnEndingChosen.Broadcast(Ending);
}

bool UHorrorGameSubsystem::TryConsumeScareBudget(float CooldownSeconds)
{
	const double Now = FPlatformTime::Seconds();
	if (Now - LastScareTime < CooldownSeconds)
	{
		return false;
	}

	LastScareTime = Now;
	return true;
}
