// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterMeshActor.generated.h"

class UProceduralMeshComponent;
struct FProcMeshTangent;
class  UBoxComponent;

USTRUCT()
struct FTrianglesSquare
{
	GENERATED_BODY()
public:
	TArray<FVector> Vertices;
	TArray<FVector2D> UV0;
public:
	FTrianglesSquare()
	{
		//First Triangle
		Vertices.Add(FVector(0, 0, 0));
		Vertices.Add(FVector(0, 1.f, 0));
		Vertices.Add(FVector(0, 0, 1.f));
		//Second Triangle
		Vertices.Add(FVector(0, 1.f, 1.f));
		//Supposed coordinates
		//Vertices.Add(FVector(0, 0, 1));
		//Vertices.Add(FVector(0, 1, 0));

		UV0.Add(FVector2D(0, 0));
		UV0.Add(FVector2D(1, 0));
		UV0.Add(FVector2D(0, 1));
		UV0.Add(FVector2D(1, 1));
	}
};

UCLASS()
class BUOYANCYPROJECT_API AWaterMeshActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWaterMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	void CreateMesh(TArray<int32>& _triangles, TArray<FVector>& _normals, TArray<FProcMeshTangent>& _tangents, TArray<FLinearColor>& _vertexColors, TArray<FVector2D>& _UV0, TArray<FVector>& _vertices);
	float GetViewDistancePercentageToSector(const FVector& sectorLocation);
	UFUNCTION()
	void OnComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* WaterMesh;
	TArray<AActor*> OverlappedActors;
public:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	int QuantityTriangles;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	int QuantitySectors;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	float BoundingSphereRadius;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	int Width;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	int Height;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	float Tolerance;
	UPROPERTY(EditAnywhere, Category = "Waves")
	float WaveHeight;
	UPROPERTY(EditAnywhere, Category = "Waves")
	float WaveLength;
	UPROPERTY(EditAnywhere, Category = "Waves")
	float Steepness;
	UPROPERTY(EditAnywhere, Category = "Water")
	UBoxComponent* WaterBoxTrigger;

};
