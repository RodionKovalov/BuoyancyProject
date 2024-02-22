// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h" 
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "Editor.h"
#include "Engine.h"
#include "GameFramework/PlayerController.h"
#include "BuoyancyComponent.h"
#include "Components/BoxComponent.h"
#include "WaterMeshActor.h"


// Sets default values
AWaterMeshActor::AWaterMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	WaterBoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterBoxTrigger"));
	WaterBoxTrigger->SetupAttachment(WaterMesh);
	RootComponent = WaterMesh;
	Width = 5000;
	Height = 5000;
	WaveHeight = 1.f;
	WaveLength = 1.f;
	Steepness = 1.f;
	QuantityTriangles = 4;
	QuantitySectors = 5;
	BoundingSphereRadius = 5000.f;
	Tolerance = 40.f;
}

// Called when the game starts or when spawned
void AWaterMeshActor::BeginPlay()
{
	Super::BeginPlay();
	WaterBoxTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWaterMeshActor::OnComponentOverlap);
	WaterBoxTrigger->OnComponentEndOverlap.AddDynamic(this, &AWaterMeshActor::OverlapEnd);

}
void AWaterMeshActor::OnComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	UBuoyancyComponent* buoyancyComponent = OtherActor->FindComponentByClass<UBuoyancyComponent>();
	if (!buoyancyComponent) return;
	OverlappedActors.Add(OtherActor);
	buoyancyComponent->SetParam(true);
}

void AWaterMeshActor::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UBuoyancyComponent* buoyancyComponent = OtherActor->FindComponentByClass<UBuoyancyComponent>();
	if (buoyancyComponent)
	{
		OverlappedActors.Remove(OtherActor);
		buoyancyComponent->SetParam(false);
	}
}
void AWaterMeshActor::PostActorCreated()
{
	Super::PostActorCreated();
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	TArray<FVector2D> UV0;
	TArray<FVector> vertices;
	CreateMesh(triangles, normals, tangents, vertexColors, UV0, vertices);
	WaterMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);
}

// This is called when actor is already in level and map is opened
void AWaterMeshActor::PostLoad()
{
	Super::PostLoad();
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	TArray<FVector2D> UV0;
	TArray<FVector> vertices;
	CreateMesh(triangles,normals,tangents,vertexColors,UV0,vertices);
	WaterMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);
}
void AWaterMeshActor::CreateMesh(TArray<int32>& _triangles, TArray<FVector>& _normals, TArray<FProcMeshTangent>& _tangents, TArray<FLinearColor>& _vertexColors, TArray<FVector2D>& _UV0, TArray<FVector>& _vertices)
{
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	TArray<FVector2D> UV0;
	TArray<FVector> vertices;
	//Sector coordinate offset
	float currentSectorOffsetZ = 0.f;
	float currentSectorOffsetY = 0.f;
	//Vertex coordinate offset
	float currentVertexOffsetZ = 0.f;
	float currentVertexOffsetY = 0.f;
	float currentSquareSizeY = 0.f;
	float currentSquareSizeZ = 0.f;
	TArray<int> sameVerticesIndecies;
	//Wave coordinate offset
	float waveOffset = 0.f;
	//Calculate avarage sum of mesh sides
	float averageSumSize = (Width + Height) / 2.f;
	float baseSectorOffset = ceil(averageSumSize / ((float)QuantitySectors));
	float baseSquareSize = ceil(baseSectorOffset / ((float)QuantityTriangles));
	for (currentSectorOffsetZ = 0.f; ceil(currentSectorOffsetZ) < Height; currentSectorOffsetZ += baseSectorOffset)
	{
		for (currentSectorOffsetY = 0.f; ceil(currentSectorOffsetY) < Width; currentSectorOffsetY += baseSectorOffset)
		{
			//Calculate center relative location of sector
			FVector relativeSectorLocation = FVector(0.f, currentSectorOffsetY - baseSectorOffset / 2.f, currentSectorOffsetZ - baseSectorOffset / 2.f);
			//Calculate center world location of sector
			FVector worldSectorLocation = WaterMesh->GetComponentTransform().TransformPosition(relativeSectorLocation);
			float squareSizeCoef = UKismetMathLibrary::MapRangeClamped(GetViewDistancePercentageToSector(worldSectorLocation), 1.f, 0.f, 1.f, 3.f);
			float evenSquareSize = baseSquareSize * squareSizeCoef;
			float oddSquareSize = baseSquareSize - abs(evenSquareSize - baseSquareSize);
			float MaxSquareSize = 2 * baseSquareSize;
			currentVertexOffsetZ = 0.f;
			currentVertexOffsetY = 0.f;
			//Calctulate Squarees in sector
			for (int squareIndexZ = 0; squareIndexZ <= QuantityTriangles - 1; squareIndexZ++)
			{
				currentVertexOffsetY = 0.f;
				currentSquareSizeZ = FMath::Clamp(((squareIndexZ % 2 == 0.f) ? evenSquareSize : oddSquareSize), 0.f, MaxSquareSize);
				sameVerticesIndecies.Empty();
				for (int squareIndexY = 0; squareIndexY <= QuantityTriangles - 1; squareIndexY++)
				{
					//Creating base Square
					FTrianglesSquare Square;
					TArray<FVector> baseVertices = Square.Vertices;
					TArray<FVector2D> baseUV = Square.UV0;
					currentSquareSizeY = FMath::Clamp(((squareIndexY % 2 == 0.f) ? evenSquareSize : oddSquareSize), 0.f, MaxSquareSize);
					for (int baseVertexIndex = 0; baseVertexIndex <= baseVertices.Num() - 1;baseVertexIndex++)
					{
						waveOffset += WaveLength;
						//If the square size is less than zero, then ignore it
						if (currentSquareSizeY <= 0.f || currentSquareSizeZ <= 0.f) continue;
						//Calculate vertex coordinates taking into account sector coordinates
						FVector* currentVertex = &baseVertices[baseVertexIndex];
						currentVertex->Y *= currentSquareSizeY;
						currentVertex->Z *= currentSquareSizeZ;
						currentVertex->Z += currentVertexOffsetZ + currentSectorOffsetZ;
						currentVertex->Y += currentVertexOffsetY + currentSectorOffsetY;
						//Calculate vertex UV coordinates taking into account sector coordinates
						baseUV[baseVertexIndex].X *= currentSquareSizeY / baseSectorOffset;
						baseUV[baseVertexIndex].Y *= currentSquareSizeZ / baseSectorOffset;
						baseUV[baseVertexIndex].Y += (currentVertexOffsetZ + currentSectorOffsetZ) / baseSectorOffset;
						baseUV[baseVertexIndex].X += (currentVertexOffsetY + currentSectorOffsetY) / baseSectorOffset;

						//Search for the same vertex
						int sameVertexIndex = -1;
						if (vertices.Num() > 0)
						{
							for (int vertexIndex = 0; vertexIndex <= vertices.Num() - 1; vertexIndex++)
							{
								bool bFindVertex = false;
								if (sameVerticesIndecies.Num() > 0)
								{
									for (int sameIndex = 0; sameIndex <= sameVerticesIndecies.Num() - 1; sameIndex++)
									{
										if (sameVerticesIndecies[sameIndex] == vertexIndex)
										{
											bFindVertex = true;
											break;
										}
									}

								}
								if (bFindVertex) continue;
								float distance = sqrt(pow(currentVertex->Z - vertices[vertexIndex].Z, 2) + pow(currentVertex->Y - vertices[vertexIndex].Y, 2));
								if (distance <= Tolerance)
								{
									//if (sameVertexIndex == -1 || (*currentVertex - Vertices[vertexIndex]).Size() < (*currentVertex - Vertices[sameVertexIndex]).Size())
									//{
									sameVertexIndex = vertexIndex;
									//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("distance: %f"), distance));
									break;
									//}


								}

							}
						}

						//found the same one
						if (sameVertexIndex != -1)
						{
							triangles.Add(sameVertexIndex);
							vertices[sameVertexIndex] = *currentVertex;
							currentVertex = &vertices[sameVertexIndex];
							sameVerticesIndecies.Add(sameVertexIndex);
						}
						//didn't find the same one
						else
						{
							normals.Add(FVector(-1, 0, 0));
							int indexVertex = vertices.Add(*currentVertex);
							sameVerticesIndecies.Add(indexVertex);
							triangles.Add(indexVertex);
							vertexColors.Add(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f));
							tangents.Add(FProcMeshTangent(0, -1, 0));
							UV0.Add(baseUV[baseVertexIndex]);
						}
						currentVertex->X += FMath::Cos(UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld()) * Steepness + waveOffset) * WaveHeight;


					}
					//Adding diagonal vertices of last Triangle
					if (triangles.Num() > 0 && currentSquareSizeY > 0.f && currentSquareSizeZ > 0.f)
					{
						int indexFirstVertex = triangles[triangles.Num() - 2];
						int indexSecondVertex = triangles[triangles.Num() - 3];
						triangles.Add(indexFirstVertex);
						triangles.Add(indexSecondVertex);
					}
					currentVertexOffsetY += currentSquareSizeY;
					sameVerticesIndecies.Empty();

				}
				currentVertexOffsetZ += currentSquareSizeZ;
			}
		}

	}
	_vertices = vertices; 
	_triangles = triangles; 
	_normals = normals;
	_UV0 = UV0;
	_vertexColors = vertexColors;
	_tangents = tangents;
}

float AWaterMeshActor::GetViewDistancePercentageToSector(const FVector& sectorLocation)
{
	float approx_screen_percentage = 1.f;

	if (!GetWorld()) return approx_screen_percentage;

	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	FVector deltaLocation = FVector::ZeroVector;
	APlayerController* controller = GetWorld()->GetFirstPlayerController();
	if (!controller) return approx_screen_percentage;
	controller->GetPlayerViewPoint(viewLocation, viewRotation);
	deltaLocation = sectorLocation - viewLocation;
	APlayerCameraManager* cameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!cameraManager) return approx_screen_percentage;

	approx_screen_percentage = FMath::Clamp(BoundingSphereRadius / (deltaLocation.Size() * FMath::Sin(cameraManager->GetFOVAngle())), 0.f, 1.f);
	return approx_screen_percentage;



}

// Called every frame
void AWaterMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	TArray<FVector2D> UV0;
	TArray<FVector> vertices;
	CreateMesh(triangles, normals, tangents, vertexColors, UV0, vertices);
	WaterMesh->ContainsPhysicsTriMeshData(true);
	WaterMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);

}

