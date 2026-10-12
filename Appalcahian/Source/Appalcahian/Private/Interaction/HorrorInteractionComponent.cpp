#include "Interaction/HorrorInteractionComponent.h"
#include "Interaction/HorrorInteractable.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UHorrorInteractionComponent::UHorrorInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHorrorInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetFocus(bInteractionEnabled ? TraceForInteractable() : nullptr);

	AActor* Held = HeldActor.Get();
	if (!Held)
	{
		return;
	}

	// Looking away or the target becoming unusable abandons the hold.
	if (Held != FocusedActor.Get())
	{
		CancelHold();
		return;
	}

	HoldElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(HoldElapsed / HoldDuration, 0.f, 1.f);
	IHorrorInteractable::Execute_OnHoldProgress(Held, GetOwner(), Alpha);
	OnHoldProgress.Broadcast(Alpha);

	if (Alpha >= 1.f)
	{
		HeldActor.Reset();
		CompleteInteraction(Held);
	}
}

void UHorrorInteractionComponent::BeginInteract()
{
	AActor* Target = FocusedActor.Get();
	if (!bInteractionEnabled || !Target || HeldActor.IsValid())
	{
		return;
	}

	const float Duration = IHorrorInteractable::Execute_GetHoldDuration(Target);
	if (Duration <= 0.f)
	{
		CompleteInteraction(Target);
		return;
	}

	HeldActor = Target;
	HoldDuration = Duration;
	HoldElapsed = 0.f;
}

void UHorrorInteractionComponent::EndInteract()
{
	CancelHold();
}

AActor* UHorrorInteractionComponent::TraceForInteractable() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	UWorld* World = GetWorld();
	if (!Controller || !World)
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(HorrorInteractTrace), false, GetOwner());
	FHitResult Hit;
	const FVector End = ViewLocation + ViewRotation.Vector() * InteractDistance;
	if (!World->LineTraceSingleByChannel(Hit, ViewLocation, End, TraceChannel, Params))
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->Implements<UHorrorInteractable>())
	{
		return nullptr;
	}

	return IHorrorInteractable::Execute_CanInteract(HitActor, GetOwner()) ? HitActor : nullptr;
}

void UHorrorInteractionComponent::SetFocus(AActor* NewFocus)
{
	AActor* OldFocus = FocusedActor.Get();
	if (OldFocus == NewFocus)
	{
		return;
	}

	if (OldFocus)
	{
		IHorrorInteractable::Execute_OnFocusChanged(OldFocus, false);
	}

	FocusedActor = NewFocus;

	FText Prompt = FText::GetEmpty();
	if (NewFocus)
	{
		IHorrorInteractable::Execute_OnFocusChanged(NewFocus, true);
		Prompt = IHorrorInteractable::Execute_GetInteractPrompt(NewFocus);
	}

	OnFocusChanged.Broadcast(NewFocus, Prompt);
}

void UHorrorInteractionComponent::CancelHold()
{
	AActor* Held = HeldActor.Get();
	HeldActor.Reset();
	HoldElapsed = 0.f;

	if (Held)
	{
		IHorrorInteractable::Execute_OnHoldProgress(Held, GetOwner(), 0.f);
		OnHoldProgress.Broadcast(0.f);
	}
}

void UHorrorInteractionComponent::CompleteInteraction(AActor* Target)
{
	IHorrorInteractable::Execute_Interact(Target, GetOwner());
	OnInteracted.Broadcast(Target);
}

void UHorrorInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelHold();
	SetFocus(nullptr);
	Super::EndPlay(EndPlayReason);
}
