// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreBuoyancy.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Water.generated.h"

class UMaterialInterface;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

UCLASS()
class BUOYANCYPROJECT_API AWater : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWater();
	virtual void Tick(float DeltaTime) override;
	UTextureRenderTarget2D* GetWaterTextureRT();
	UMaterialInterface* GetWaterMaterialAlpha();
	FColor GetRenderTargetValue(float x, float y, UTextureRenderTarget2D* renderTarget);
	void UpdateBuffer(UTextureRenderTarget2D* renderTarget);
	void ReadPixelsAsync(TArray< FColor >& OutImageData, UTextureRenderTarget2D* renderTexture);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void StartRenderWaterAlpha();
	void StopRenderWaterAlpha();
	void RenderWaterAlpha(UObject* worldContextObject, UTextureRenderTarget2D* waterRenderTarget, UMaterialInterface* waterMaterialAlpha);
	
public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* WaterMesh;
	UPROPERTY(EditAnywhere)
	USceneComponent* Root;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* WaterAlphaMesh;
	UPROPERTY(EditAnywhere)
	float WaterDensity;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* WaterMaterialAlpha;
	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* WaterTextureRT;
	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* SceneCapture;

	bool bReadPixelsStarted = false;
	FRenderCommandFence ReadPixelFence;
	TArray<FColor> ColorBuffer;

protected:
	bool bRenderWaterAlpha;
	TArray<AActor*> OverlappedActors;
};
