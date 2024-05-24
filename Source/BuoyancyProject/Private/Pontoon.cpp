// Fill out your copyright notice in the Description page of Project Settings.


#include "Pontoon.h"

// Sets default values for this component's properties
UPontoon::UPontoon()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Radius = 0.f;
	// ...
}


// Called when the game starts
void UPontoon::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPontoon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

