// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine/TextureRenderTarget2D.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Water.h"
#include "BuoyancyComponent.h"

// Sets default values for this component's properties
UBuoyancyComponent::UBuoyancyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Density = 100.f;
	WaveFactorCoef = 1.f;
	MinWaveFactorCoef = 0.8f;
	MaxWaveFactorCoef = 2.f;
	FormDragCoefficient = 0.1f;
	bInWater = false;
	bShowDebugLines = false;
	// ...
}


// Called when the game starts
void UBuoyancyComponent::BeginPlay()
{
	Super::BeginPlay();
	Mesh = Cast<UStaticMeshComponent>(GetOwner()->GetRootComponent());
	// ...

}

// Called every frame
void UBuoyancyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bInWater)
	{
		for (auto pontoon : Pontoons)
		{
			FHitResult hitResult;
			FHitResult hitResultComplexTrace;
			FVector worldPontoonLocation;
			FVector startLocation = worldPontoonLocation;
			FVector endLocation = worldPontoonLocation;
			float archimedesForce;
			float viscousFriction;
			FVector forceLocation;
			float volume;
			float volumeInWater;
			float volumeInWaterAlpha;
			float waveFactor = 1.f;
			FVector linearVelocity;
			FVector linearVelocity_normalize;
			FCollisionQueryParams params;
			params.AddIgnoredActor(GetOwner());
			FCollisionQueryParams paramsComplexTrace;
			paramsComplexTrace.AddIgnoredActor(GetOwner());
			paramsComplexTrace.bTraceComplex = true;
			paramsComplexTrace.bReturnFaceIndex = true;
			if (Mesh && Water)
			{
				worldPontoonLocation = Mesh->GetComponentTransform().TransformPosition(pontoon.RelativeLocation);
				startLocation = worldPontoonLocation;
				endLocation = worldPontoonLocation;
				startLocation.Z = startLocation.Z + pontoon.Radius;
				endLocation.Z = endLocation.Z - pontoon.Radius;
				hitResult = CalculateLineTrace(startLocation, endLocation, params);
				volume = CalculateVolume(Mesh->GetMass(), Density);
				volumeInWaterAlpha = CalctulateAlphaVolumeInWater(hitResult.Distance, pontoon.Radius);
				volumeInWater = CalculateVolumeInWater(volumeInWaterAlpha, volume);
				CalculateArchimedesForce(archimedesForce, volumeInWater, Water->WaterDensity, Pontoons.Num());
				// Calculating force location for ViscousFrictionForce
				forceLocation = CalculateForceLocation(Mesh->GetComponentTransform().TransformPosition(pontoon.RelativeLocation), pontoon.Radius, volumeInWaterAlpha);
				linearVelocity = Mesh->GetPhysicsLinearVelocityAtPoint(forceLocation);
				CalculateViscousFriction(viscousFriction, linearVelocity, Water->WaterDensity, volumeInWater, FormDragCoefficient, Pontoons.Num());
				// Viscous friction force acts tangentially on an object
				linearVelocity_normalize = linearVelocity;
				linearVelocity_normalize.Normalize();
				Mesh->AddForceAtLocation(viscousFriction * -linearVelocity_normalize, forceLocation);
				hitResultComplexTrace = CalculateLineTrace(startLocation, endLocation, paramsComplexTrace);
				waveFactor = CalculateWaveFactor(hitResultComplexTrace, Water->GetWaterTextureRT(), WaveFactorCoef, MinWaveFactorCoef, MaxWaveFactorCoef);

				// Accounting for wave movement
				archimedesForce *= waveFactor;
				// Archimedes' force acts on the bottom of the object
				Mesh->AddForceAtLocation(archimedesForce * hitResult.Normal, endLocation);
			}
		}
	}
}

float UBuoyancyComponent::CalctulateAlphaVolumeInWater(const float& distance, const float& pontoonRadius)
{
	return UKismetMathLibrary::MapRangeClamped(distance, pontoonRadius * 2.f, 0.f, 0.f, 1.f); // Calculating the area of object immersed in water;
}

float UBuoyancyComponent::CalculateVolume(const float& mass, const float& density)
{
	return  mass / density;
}
float UBuoyancyComponent::CalculateVolumeInWater(const float& alphaVolumeInWater, const float& volume)
{
	return volume * alphaVolumeInWater;
}
FVector UBuoyancyComponent::CalculateForceLocation(const FVector& location, const float& radius, const float& alphaVolumeInWater)
{
	FVector newLocation = location;
	newLocation.Z += UKismetMathLibrary::MapRangeClamped(alphaVolumeInWater / 2, 0.5f, 0.f, 0.f, -radius);
	return  newLocation;
}

void UBuoyancyComponent::CalculateViscousFriction(float& outViscousFriction, const FVector& velocity, const float& waterDensity, const float& volumeInWater, const float& formDragCoefficient, const int& pontoonsQuantity)
{
	float viscousFriction;
	float volume = FMath::Pow(volumeInWater / pontoonsQuantity, 2 / 3);
	viscousFriction = ((waterDensity * (velocity.Size() / 100)) / 2) * formDragCoefficient * volume;
	viscousFriction *= 100.f; // Conversion to unreal units
	outViscousFriction = viscousFriction;
}

void UBuoyancyComponent::SetParam(const bool& _bInWater, AWater* _water)
{
	bInWater = _bInWater;
	Water = _water;
}

bool UBuoyancyComponent::InWater()
{
	return bInWater;
}

FHitResult UBuoyancyComponent::CalculateLineTrace(FVector& startLocation, FVector& endLocation, FCollisionQueryParams& params)
{
	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, startLocation, endLocation, ECollisionChannel::ECC_Visibility, params);
	if (bShowDebugLines)
		DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Blue, false, 2, -1, 5);
	return hitResult;
}
float UBuoyancyComponent::CalculateWaveFactor(const FHitResult& hitResult, UTextureRenderTarget2D* textureRT, const float& waveFactorCoef, const float& minWaveFactorCoef, const float& maxWaveFactorCoef)
{
	if (!textureRT && !GetWorld()) return 1.f;
	FVector2D UV = FVector2D::ZeroVector;
	FLinearColor color = FLinearColor::Black;
	float waveFactor = 0.0f;
	FVector colorVector = FVector::ZeroVector;
	UGameplayStatics::FindCollisionUV(hitResult, 1, UV);
	color = Water->GetRenderTargetValue(UV.X, UV.Y, textureRT);//UKismetRenderingLibrary::ReadRenderTargetRawUV(GetWorld(), textureRT, UV.X, UV.Y);
	colorVector = UKismetMathLibrary::Conv_LinearColorToVector(color);
	waveFactor = waveFactorCoef * colorVector.Size();
	waveFactor = UKismetMathLibrary::FClamp(waveFactor, minWaveFactorCoef, maxWaveFactorCoef);
	return waveFactor;
}
void UBuoyancyComponent::CalculateArchimedesForce(float& outArchimedesForce, const float& volumeInWater, const float& waterDensity, const int& pontoonsQuantity)
{
	float archimedesForce = waterDensity * 9.81 * (volumeInWater / pontoonsQuantity);
	archimedesForce *= 100.f; // Conversion to unreal units
	outArchimedesForce = archimedesForce;
}
