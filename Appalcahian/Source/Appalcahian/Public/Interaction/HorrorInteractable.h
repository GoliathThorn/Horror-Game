#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HorrorInteractable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UHorrorInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Anything the player can use: notes, terminals, fuses, valves, the final seal.
 * Add it to a Blueprint via Class Settings > Implemented Interfaces and override what you need.
 */
class APPALCAHIAN_API IHorrorInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractPrompt();

	/** 0 = single press. Above 0 = the player has to hold for this many seconds (valves, the seal). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetHoldDuration();

	/** Called every frame while held, Alpha 0-1. Called with 0 if the hold is abandoned. Drive valve rotation from this. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnHoldProgress(AActor* Interactor, float Alpha);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnFocusChanged(bool bFocused);

	// Defaults for C++ implementers; Blueprints override the events above instead.
	virtual bool CanInteract_Implementation(AActor* Interactor) { return true; }
	virtual FText GetInteractPrompt_Implementation() { return FText::GetEmpty(); }
	virtual float GetHoldDuration_Implementation() { return 0.f; }
	virtual void OnHoldProgress_Implementation(AActor* Interactor, float Alpha) {}
	virtual void Interact_Implementation(AActor* Interactor) {}
	virtual void OnFocusChanged_Implementation(bool bFocused) {}
};
