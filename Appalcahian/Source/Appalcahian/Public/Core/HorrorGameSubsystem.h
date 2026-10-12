#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HorrorGameSubsystem.generated.h"

UENUM(BlueprintType)
enum class EHorrorEnding : uint8
{
	None,
	Run,
	Fight,
	LockItIn
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHorrorFlagChanged, FName, Flag, bool, bIsSet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHorrorEndingChosen, EHorrorEnding, Ending);

/**
 * Game-wide story state. Lives for the whole session, survives level loads.
 * Flags are free-form names, e.g. "Fuse_Ventilation", "ServerRoomScareDone", "ReactorRestartFailed".
 */
UCLASS()
class APPALCAHIAN_API UHorrorGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UHorrorGameSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Horror|Flags")
	void SetFlag(FName Flag, bool bValue = true);

	UFUNCTION(BlueprintPure, Category = "Horror|Flags")
	bool HasFlag(FName Flag) const;

	UFUNCTION(BlueprintCallable, Category = "Horror|Ending")
	void ChooseEnding(EHorrorEnding NewEnding);

	UFUNCTION(BlueprintPure, Category = "Horror|Ending")
	EHorrorEnding GetEnding() const { return Ending; }

	/**
	 * Shared cooldown for ambient scares (reflection flashes etc.) so several surfaces
	 * can't fire back to back. Returns true and starts the cooldown if a scare is allowed now.
	 */
	UFUNCTION(BlueprintCallable, Category = "Horror|Scares")
	bool TryConsumeScareBudget(float CooldownSeconds);

	UPROPERTY(BlueprintAssignable, Category = "Horror|Flags")
	FOnHorrorFlagChanged OnFlagChanged;

	UPROPERTY(BlueprintAssignable, Category = "Horror|Ending")
	FOnHorrorEndingChosen OnEndingChosen;

private:
	UPROPERTY()
	TSet<FName> Flags;

	EHorrorEnding Ending = EHorrorEnding::None;

	double LastScareTime = -1.0e9;
};
