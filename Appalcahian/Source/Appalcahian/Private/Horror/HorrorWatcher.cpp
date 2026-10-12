#include "Horror/HorrorWatcher.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AHorrorWatcher::AHorrorWatcher()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SightPoints.Add(FVector(0.f, 0.f, 120.f));
	SightPoints.Add(FVector(0.f, 0.f, 180.f));
}

void AHorrorWatcher::BeginPlay()
{
	Super::BeginPlay();

	if (bStartActive)
	{
		ActivateWatcher();
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
	}
}

void AHorrorWatcher::ActivateWatcher()
{
	ResetSightState();
	bWatcherActive = true;
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
}

void AHorrorWatcher::Vanish()
{
	if (!bWatcherActive)
	{
		return;
	}

	bWatcherActive = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	OnVanished.Broadcast();

	if (bDestroyOnVanish)
	{
		Destroy();
	}
}

void AHorrorWatcher::ResetSightState()
{
	bSeen = false;
	bEverSeen = false;
	bLookedAway = false;
	SeenTime = 0.f;
	UnseenTime = 0.f;
	TimeSinceReposition = 0.f;
}

void AHorrorWatcher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bWatcherActive)
	{
		return;
	}

	TimeSinceReposition += DeltaSeconds;

	if (bFacePlayer)
	{
		FacePlayer();
	}

	const bool bSeenNow = IsVisibleToPlayer();

	if (bSeenNow)
	{
		SeenTime += DeltaSeconds;
		UnseenTime = 0.f;

		if (!bSeen)
		{
			bSeen = true;
			if (!bEverSeen)
			{
				bEverSeen = true;
				OnFirstSeen.Broadcast();
			}
			else if (bLookedAway)
			{
				bLookedAway = false;
				OnLookedBackAt.Broadcast();
			}
		}
		return;
	}

	UnseenTime += DeltaSeconds;
	if (!bEverSeen || UnseenTime < UnseenGraceTime)
	{
		// Accompany doesn't wait to be glimpsed first: it starts trailing immediately.
		if (Behavior == EWatcherBehavior::Accompany && !bEverSeen && UnseenTime >= UnseenGraceTime)
		{
			TryRepositionBehindPlayer();
		}
		return;
	}

	if (bSeen)
	{
		bSeen = false;
		bLookedAway = true;
		OnLostSight.Broadcast();
	}

	switch (Behavior)
	{
	case EWatcherBehavior::VanishWhenUnseen:
		if (SeenTime >= MinSeenTime)
		{
			Vanish();
		}
		break;

	case EWatcherBehavior::StayWhenUnseen:
		break;

	case EWatcherBehavior::Accompany:
		TryRepositionBehindPlayer();
		break;
	}
}

bool AHorrorWatcher::IsVisibleToPlayer() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!World || !PC)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewForward = ViewRotation.Vector();
	const float CosLimit = FMath::Cos(FMath::DegreesToRadians(ViewConeHalfAngle));

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WatcherSight), false, this);
	if (const APawn* PlayerPawn = PC->GetPawn())
	{
		Params.AddIgnoredActor(PlayerPawn);
	}

	const FTransform& Transform = GetActorTransform();
	for (const FVector& LocalPoint : SightPoints)
	{
		const FVector Point = Transform.TransformPosition(LocalPoint);
		const FVector ToPoint = Point - ViewLocation;
		const float Distance = ToPoint.Size();
		if (Distance < KINDA_SMALL_NUMBER || Distance > MaxSightDistance)
		{
			continue;
		}

		if (FVector::DotProduct(ToPoint / Distance, ViewForward) < CosLimit)
		{
			continue;
		}

		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, ViewLocation, Point, ECC_Visibility, Params))
		{
			return true;
		}
	}

	return false;
}

void AHorrorWatcher::FacePlayer()
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ToPlayer = ViewLocation - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (!ToPlayer.IsNearlyZero())
	{
		SetActorRotation(FRotator(0.f, ToPlayer.Rotation().Yaw, 0.f));
	}
}

bool AHorrorWatcher::TryRepositionBehindPlayer()
{
	if (TimeSinceReposition < AccompanyRepositionInterval)
	{
		return false;
	}

	UWorld* World = GetWorld();
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	if (!World || !PlayerPawn)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector Back = -FRotator(0.f, ViewRotation.Yaw, 0.f).Vector();
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const FVector Desired = PlayerLocation + Back * AccompanyDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(WatcherReposition), false, this);
	Params.AddIgnoredActor(PlayerPawn);

	// Don't put it through a wall: there must be open space between the player and the spot.
	FHitResult WallHit;
	if (World->LineTraceSingleByChannel(WallHit, PlayerLocation, Desired, ECC_Visibility, Params))
	{
		return false;
	}

	// Find the floor under the spot.
	FHitResult FloorHit;
	const FVector FloorStart = Desired + FVector(0.f, 0.f, 150.f);
	const FVector FloorEnd = Desired - FVector(0.f, 0.f, 600.f);
	if (!World->LineTraceSingleByChannel(FloorHit, FloorStart, FloorEnd, ECC_Visibility, Params))
	{
		return false;
	}

	const FVector OldLocation = GetActorLocation();
	SetActorLocation(FloorHit.ImpactPoint);

	// Never pop into view.
	if (IsVisibleToPlayer())
	{
		SetActorLocation(OldLocation);
		return false;
	}

	TimeSinceReposition = 0.f;
	return true;
}
