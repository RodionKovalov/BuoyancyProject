// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreBuoyancy.h"
#include "Components/ActorComponent.h"
#include "Pontoon.h"
#include "BuoyancyComponent.generated.h"


class UStaticMeshComponent;
class UPontoon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUOYANCYPROJECT_API UBuoyancyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float deltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UBuoyancyComponent();
	void SetParam(bool _bInWater);
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
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	TArray<UPontoon*> Pontoons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float Density;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float FormDragCoefficient;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	float WaterDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buoyancy")
	bool bShowDebugLines;
protected:
	bool bInWater;
	UStaticMeshComponent* Mesh;
	float DeltaTime;

};
