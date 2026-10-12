#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "HorrorInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractFocusChanged, AActor*, FocusedActor, FText, Prompt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractHoldProgress, float, Alpha);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, AActor*, Target);

/**
 * Looks down the camera each frame for something implementing IHorrorInteractable.
 * Bind your Interact input: Started -> BeginInteract, Completed/Canceled -> EndInteract.
 * Bind OnFocusChanged to show/hide the prompt widget.
 */
UCLASS(ClassGroup = (Horror), meta = (BlueprintSpawnableComponent))
class APPALCAHIAN_API UHorrorInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHorrorInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void BeginInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsHolding() const { return HeldActor.IsValid(); }

	/** Reach in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractDistance = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Turn off during cutscenes / control locks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractHoldProgress OnHoldProgress;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteracted OnInteracted;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	AActor* TraceForInteractable() const;
	void SetFocus(AActor* NewFocus);
	void CancelHold();
	void CompleteInteraction(AActor* Target);

	TWeakObjectPtr<AActor> FocusedActor;
	TWeakObjectPtr<AActor> HeldActor;
	float HoldElapsed = 0.f;
	float HoldDuration = 0.f;
};
