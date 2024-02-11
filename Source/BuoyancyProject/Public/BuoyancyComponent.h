// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreBuoyancy.h"
#include "Components/ActorComponent.h"
#include "BuoyancyComponent.generated.h"


class UTextureRenderTarget2D;
class UStaticMeshComponent;
class AWater;

USTRUCT(BlueprintType)
struct FPontoonStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FVector RelativeLocation;
	UPROPERTY(EditAnywhere)
	float Radius = 0.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUOYANCYPROJECT_API UBuoyancyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UBuoyancyComponent();
	void SetParam(const bool& _bInWater, AWater* _water);
	bool InWater();
protected:
	virtual void BeginPlay() override;
	//Calculating Volume
	float CalctulateAlphaVolumeInWater(const float& distance, const float& pontoonRadius);
	float CalculateVolume(const float& mass, const float& density);
	float CalculateVolumeInWater(const float& alphaVolumeInWater, const float& volume);

	FHitResult CalculateLineTrace(FVector& startLocation, FVector& endLocation, FCollisionQueryParams& params);
	void CalculateArchimedesForce(float& outArchimedesForce, const float& volumeInWater, const float& waterDensity, const int& pontoonsQuantity);
	// Calculating force location for ViscousFrictionForce
	FVector CalculateForceLocation(const FVector& location, const float& radius, const float& alphaVolumeInWater);
	void CalculateViscousFriction(float& outViscousFriction, const FVector& velocity, const float& waterDensity, const float& volumeInWater, const float& formDragCoefficient, const int& pontoonsQuantity);
	float CalculateWaveFactor(const FHitResult& hitResult, UTextureRenderTarget2D* textureRT, const float& waveFactorCoef, const float& minWaveFactorCoef, const float& maxWaveFactorCoef);
public:
	UPROPERTY(EditAnywhere)
	TArray<FPontoonStruct> Pontoons;
	UPROPERTY(EditAnywhere)
	float Density;
	UPROPERTY(EditAnywhere)
	float FormDragCoefficient;
	UPROPERTY(EditAnywhere, Category = "WaterFactor")
	float WaveFactorCoef;
	UPROPERTY(EditAnywhere, Category = "WaterFactor")
	float MinWaveFactorCoef;
	UPROPERTY(EditAnywhere, Category = "WaterFactor")
	float MaxWaveFactorCoef;
	UPROPERTY(EditAnywhere)
	bool bShowDebugLines;
protected:
	AWater* Water;
	bool bInWater;
	UStaticMeshComponent* Mesh;

};
