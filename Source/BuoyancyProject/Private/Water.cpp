// Fill out your copyright notice in the Description page of Project Settings.

#include "RHICommandList.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "BuoyancyComponent.h"
#include "Water.h"

// Sets default values
AWater::AWater()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water Mesh"));
	WaterAlphaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterAlphaMesh"));
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	WaterMesh->SetupAttachment(RootComponent);
	SceneCapture->SetupAttachment(RootComponent);
	WaterAlphaMesh->SetupAttachment(RootComponent);
	SceneCapture->bAutoActivate = false;
	SceneCapture->ProjectionType = ECameraProjectionMode::Orthographic;
	WaterMesh->SetCollisionProfileName("Water");
	WaterAlphaMesh->SetCollisionProfileName("WaterAlpha");
	WaterAlphaMesh->bVisibleInSceneCaptureOnly = true;
	WaterDensity = 977.f;
	bRenderWaterAlpha = false;
}

// Called when the game starts or when spawned
void AWater::BeginPlay()
{
	Super::BeginPlay();
	WaterMesh->OnComponentBeginOverlap.AddDynamic(this, &AWater::OnComponentOverlap);
	WaterMesh->OnComponentEndOverlap.AddDynamic(this, &AWater::OverlapEnd);
}

void AWater::OnComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	UBuoyancyComponent* buoyancyComponent = OtherActor->FindComponentByClass<UBuoyancyComponent>();
	if (!buoyancyComponent) return;
	OverlappedActors.Add(OtherActor);
	buoyancyComponent->SetParam(true);
	StartRenderWaterAlpha();
}

void AWater::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UBuoyancyComponent* buoyancyComponent = OtherActor->FindComponentByClass<UBuoyancyComponent>();
	if (buoyancyComponent)
	{
		OverlappedActors.Remove(OtherActor);
		buoyancyComponent->SetParam(false);
		TArray<AActor*> actors;
		WaterMesh->GetOverlappingActors(actors);
		if (actors.Num() <= 0)
		{
			StopRenderWaterAlpha();
		}
	}
}

void AWater::StartRenderWaterAlpha()
{
	SceneCapture->bAutoActivate = true;
	SceneCapture->TextureTarget = GetWaterTextureRT();
	WaterAlphaMesh->SetStaticMesh(WaterMesh->GetStaticMesh());
	WaterAlphaMesh->SetRelativeScale3D(WaterMesh->GetRelativeScale3D());
	WaterAlphaMesh->SetMaterial(0, WaterMaterialAlpha);
	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator(-90.f, -90.f, 0.f);
	float waterSquare;
	waterSquare = WaterAlphaMesh->GetRelativeScale3D().X * WaterAlphaMesh->GetRelativeScale3D().Y;
	location.Z = waterSquare / 2.f;
	SceneCapture->OrthoWidth = waterSquare;
	SceneCapture->SetRelativeLocation(location);
	SceneCapture->SetRelativeRotation(rotation);
	SceneCapture->ShowOnlyComponent(WaterAlphaMesh);
	bRenderWaterAlpha = true;
}

void AWater::StopRenderWaterAlpha()
{
	bRenderWaterAlpha = false;
	SceneCapture->bAutoActivate = false;
}

void AWater::RenderWaterAlpha(UObject* worldContextObject, UTextureRenderTarget2D* waterRenderTarget, UMaterialInterface* waterMaterialAlpha)
{
	if (worldContextObject && waterRenderTarget && waterMaterialAlpha)
	{
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(worldContextObject, waterRenderTarget, waterMaterialAlpha);
	}
}
UTextureRenderTarget2D* AWater::GetWaterTextureRT()
{
	return WaterTextureRT;
}
UMaterialInterface* AWater::GetWaterMaterialAlpha()
{
	return WaterMaterialAlpha;
}
// Called every frame
void AWater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (OverlappedActors.Num() > 0)
	{
		if (bRenderWaterAlpha)
		{
			if (!bReadPixelsStarted)
			{
				UpdateBuffer(GetWaterTextureRT());
			}
			if (bReadPixelsStarted && ReadPixelFence.IsFenceComplete())
			{
				bReadPixelsStarted = false;
			}
		}

	}

}
void AWater::UpdateBuffer(UTextureRenderTarget2D* renderTarget)
{
	//ColorBuffer.Reset();

	if (renderTarget != nullptr)
	{
		ReadPixelsAsync(ColorBuffer, WaterTextureRT);
		ReadPixelFence.BeginFence();
		bReadPixelsStarted = true;
	}
}
FColor AWater::GetRenderTargetValue(float x, float y, UTextureRenderTarget2D* renderTarget)
{
	float size = 10000;

	if (renderTarget == nullptr || ColorBuffer.Num() == 0)
		return FColor(0);

	float width = renderTarget->GetSurfaceWidth();
	float height = renderTarget->GetSurfaceHeight();

	//Conver coordinates to texture space
	float normalizedX = (x / size) + 0.5f;
	float normalizedY = (y / size) + 0.5f;

	int i = (int)(normalizedX * width);
	int j = (int)(normalizedY * height);

	if (i < 0) i = 0;
	if (i >= width) i = width - 1;
	if (j < 0) j = 0;
	if (j >= height) j = height - 1;

	int index = i + j * width;
	if (index < 0) index = 0;
	if (index >= ColorBuffer.Num()) index = ColorBuffer.Num();

	return ColorBuffer[index];
}

void AWater::ReadPixelsAsync(TArray< FColor >& OutImageData, UTextureRenderTarget2D* renderTexture)
{

	FTextureRenderTarget2DResource* RenderResource = (FTextureRenderTarget2DResource*)renderTexture->Resource;

	// Read the render target surface data back.	
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	OutImageData.Reset();
	FReadSurfaceContext ReadSurfaceContext =
	{
		RenderResource,
		&OutImageData,
		FIntRect(0, 0, RenderResource->GetSizeXY().X, RenderResource->GetSizeXY().Y),
		FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)
	};

	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[ReadSurfaceContext](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceData(
				ReadSurfaceContext.SrcRenderTarget->GetRenderTargetTexture(),
				ReadSurfaceContext.Rect,
				*ReadSurfaceContext.OutData,
				ReadSurfaceContext.Flags
			);
		});
}