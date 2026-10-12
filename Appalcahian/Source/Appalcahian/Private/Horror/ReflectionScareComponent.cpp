#include "Horror/ReflectionScareComponent.h"
#include "Core/HorrorGameSubsystem.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

UReflectionScareComponent::UReflectionScareComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReflectionScareComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!SurfaceMaterial)
	{
		if (UMeshComponent* Mesh = GetOwner() ? GetOwner()->FindComponentByClass<UMeshComponent>() : nullptr)
		{
			SetTargetMesh(Mesh, 0);
		}
	}
}

void UReflectionScareComponent::SetTargetMesh(UMeshComponent* Mesh, int32 MaterialIndex)
{
	SurfaceMaterial = Mesh ? Mesh->CreateAndSetMaterialInstanceDynamic(MaterialIndex) : nullptr;
	SetEyes(0.f);
}

void UReflectionScareComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FramesRemaining > 0)
	{
		if (--FramesRemaining == 0)
		{
			SetEyes(0.f);
		}
		return;
	}

	const bool bLooking = IsPlayerLookingAtOwner();
	const bool bNewLook = bLooking && !bWasLooking;
	bWasLooking = bLooking;

	if (!bNewLook)
	{
		return;
	}

	UHorrorGameSubsystem* Game = UHorrorGameSubsystem::Get(this);
	if (!Game)
	{
		return;
	}

	if (!RequiredFlag.IsNone() && !Game->HasFlag(RequiredFlag))
	{
		return;
	}

	// Roll first so a failed roll doesn't eat the shared cooldown.
	if (FMath::FRand() < Chance && Game->TryConsumeScareBudget(SharedCooldown))
	{
		ForceScare();
	}
}

void UReflectionScareComponent::ForceScare()
{
	FramesRemaining = FMath::Max(1, FlashFrames);
	SetEyes(1.f);
	OnScare.Broadcast();
}

bool UReflectionScareComponent::IsPlayerLookingAtOwner() const
{
	const AActor* Owner = GetOwner();
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!Owner || !PC)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector Origin;
	FVector Extent;
	Owner->GetActorBounds(true, Origin, Extent);

	const FVector ToSurface = Origin - ViewLocation;
	const float Distance = ToSurface.Size();
	if (Distance < KINDA_SMALL_NUMBER || Distance > MaxLookDistance)
	{
		return false;
	}

	const float CosLimit = FMath::Cos(FMath::DegreesToRadians(LookConeHalfAngle));
	return FVector::DotProduct(ToSurface / Distance, ViewRotation.Vector()) >= CosLimit;
}

void UReflectionScareComponent::SetEyes(float Value)
{
	if (SurfaceMaterial)
	{
		SurfaceMaterial->SetScalarParameterValue(EyesParameterName, Value);
	}
}
