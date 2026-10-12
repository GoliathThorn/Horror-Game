#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HorrorWatcher.generated.h"

class USceneComponent;

UENUM(BlueprintType)
enum class EWatcherBehavior : uint8
{
	/** The trained rule (Acts 2-5.1). Once glimpsed, looking away or losing line of sight makes it gone. */
	VanishWhenUnseen,
	/** The rule breaks (Act 5.3). Still there when you look back; fires OnLookedBackAt so you can hold the beat. */
	StayWhenUnseen,
	/** Act 5.4 onward. Whenever unseen, quietly repositions behind the player at a fixed distance. */
	Accompany
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWatcherEvent);

/**
 * The creature as a presence, not an enemy. No AI, no attacks, no pathing: it only reacts to
 * whether the player can see it. Make a Blueprint child, add the silhouette mesh, keep the actor
 * origin at its feet, and give the mesh no collision.
 *
 * "Seen" = inside the central view cone and an unblocked line from the camera to at least one
 * SightPoint. It does not check lighting.
 */
UCLASS(Blueprintable)
class APPALCAHIAN_API AHorrorWatcher : public AActor
{
	GENERATED_BODY()

public:
	AHorrorWatcher();

	virtual void Tick(float DeltaSeconds) override;

	/** Show it and start watching. Use with bStartActive off to place watchers that wait for a trigger. */
	UFUNCTION(BlueprintCallable, Category = "Watcher")
	void ActivateWatcher();

	/** Gone, immediately. Hidden, collision off, ticking off. */
	UFUNCTION(BlueprintCallable, Category = "Watcher")
	void Vanish();

	UFUNCTION(BlueprintCallable, Category = "Watcher")
	void SetBehavior(EWatcherBehavior NewBehavior) { Behavior = NewBehavior; }

	UFUNCTION(BlueprintPure, Category = "Watcher")
	bool IsSeen() const { return bSeen; }

	UFUNCTION(BlueprintPure, Category = "Watcher")
	bool IsWatcherActive() const { return bWatcherActive; }

	/** Can the player see it right now. Exposed for scripting (e.g. the 5.2 door only closes while watched). */
	UFUNCTION(BlueprintPure, Category = "Watcher")
	bool IsVisibleToPlayer() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher")
	EWatcherBehavior Behavior = EWatcherBehavior::VanishWhenUnseen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher")
	bool bStartActive = true;

	/** Keep turning to face the camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher")
	bool bFacePlayer = true;

	/** Local-space points tested for line of sight. Defaults: chest and head height. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher|Sight", meta = (MakeEditWidget = true))
	TArray<FVector> SightPoints;

	/** Degrees off screen-center that still counts as looking at it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Sight", meta = (ClampMin = "1", ClampMax = "90"))
	float ViewConeHalfAngle = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Sight", meta = (ClampMin = "100"))
	float MaxSightDistance = 6000.f;

	/** It has to be seen this long before it's allowed to vanish, so a one-frame edge glimpse doesn't burn the beat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Timing", meta = (ClampMin = "0"))
	float MinSeenTime = 0.2f;

	/** How long it must be out of sight before it counts as "looked away". Keep short so a pillar passing (<1s) is enough. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Timing", meta = (ClampMin = "0"))
	float UnseenGraceTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher")
	bool bDestroyOnVanish = false;

	/** Accompany: distance kept behind the player, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Accompany", meta = (ClampMin = "100"))
	float AccompanyDistance = 450.f;

	/** Accompany: minimum seconds between repositions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watcher|Accompany", meta = (ClampMin = "0"))
	float AccompanyRepositionInterval = 0.75f;

	/** Seen for the first time since activation. */
	UPROPERTY(BlueprintAssignable, Category = "Watcher")
	FOnWatcherEvent OnFirstSeen;

	/** Went from seen to unseen (after the grace time). */
	UPROPERTY(BlueprintAssignable, Category = "Watcher")
	FOnWatcherEvent OnLostSight;

	/** Seen again after the player looked away. The rule-break beat in StayWhenUnseen. */
	UPROPERTY(BlueprintAssignable, Category = "Watcher")
	FOnWatcherEvent OnLookedBackAt;

	UPROPERTY(BlueprintAssignable, Category = "Watcher")
	FOnWatcherEvent OnVanished;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Watcher")
	TObjectPtr<USceneComponent> Root;

private:
	void ResetSightState();
	void FacePlayer();
	bool TryRepositionBehindPlayer();

	bool bWatcherActive = false;
	bool bSeen = false;
	bool bEverSeen = false;
	bool bLookedAway = false;
	float SeenTime = 0.f;
	float UnseenTime = 0.f;
	float TimeSinceReposition = 0.f;
};
