#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlashlightProfile.generated.h"

/**
 * One flashlight "feel". Make two data assets from this:
 *  DA_Flashlight_GodBeam - wide, ~6500K, bright, no flicker (Acts 1-4.1)
 *  DA_Flashlight_DCell   - ~35 deg, ~2800K, dim, slight flicker (Act 4.2 onward)
 */
UCLASS(BlueprintType)
class APPALCAHIAN_API UFlashlightProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Full cone angle in degrees (UE spot lights use the half angle; we convert). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "2", ClampMax = "160"))
	float ConeAngle = 70.f;

	/** How much of the cone is full brightness before falloff, 0-1. Lower = softer edge. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "0", ClampMax = "1"))
	float HotspotRatio = 0.5f;

	/** Candelas. Tune against your exposure settings in-editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "0"))
	float Intensity = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "1700", ClampMax = "12000"))
	float TemperatureKelvin = 6500.f;

	/** How far the light reaches, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "100"))
	float AttenuationRadius = 4000.f;

	/** How strongly the beam lights up volumetric fog. Crank this for the D-cell in the tunnels. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beam", meta = (ClampMin = "0"))
	float VolumetricScattering = 1.f;

	/** 0 = rock steady, 1 = can dip to black. Keep the D-cell around 0.1-0.2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flicker", meta = (ClampMin = "0", ClampMax = "1"))
	float FlickerStrength = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flicker", meta = (ClampMin = "0"))
	float FlickerSpeed = 8.f;
};
