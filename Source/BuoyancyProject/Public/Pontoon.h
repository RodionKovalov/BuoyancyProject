// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Pontoon.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUOYANCYPROJECT_API UPontoon : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPontoon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	UPROPERTY(EditAnywhere)
	float Radius;
		
};
