// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EvoStructs.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "AssetSpawnerVenice.h"
#include "EvoMapGenerator.h"
#include "EvoVenice.generated.h"

UCLASS()
class EVOLUTIONARYMAPS_API AEvoVenice : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEvoVenice();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere)
	int Width = 64;

	UPROPERTY(EditAnywhere)
	int Height = 64;

	UPROPERTY(EditAnywhere = "Algorithm Params")
	bool bTickMode = false;
	UPROPERTY(EditAnywhere = "Algorithm Params")
	int32 MaximumIterations = 1000;
	UPROPERTY(EditAnywhere = "Algorithm Params")
	int32 MutationsPerIteration = 20;

	UPROPERTY(EditAnywhere, Category = "Evaluation Params")
	int32 TargetStreetTiles = 800;
	UPROPERTY(EditAnywhere, Category = "Evaluation Params")
	int32 TargetCanalTiles = 400;
	UPROPERTY(EditAnywhere, Category = "Evaluation Params")
	int32 TargetStartStartDistance = 40;
	UPROPERTY(EditAnywhere, Category = "Evaluation Params")
	int32 TargetStartDestinationDistance = 60;


	UPROPERTY(EditAnywhere)
	TSubclassOf<UEvoMapGenerator> MapGenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AAssetSpawnerVenice> AssetSpawnerClass;

	UPROPERTY()
	UEvoMapGenerator* MapGen;

	UPROPERTY()
	AAssetSpawnerVenice* AssetSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	UTextureRenderTarget2D* RenderTargetAsset;

	TArray<FEvoGraph> EvoGraphs;


	void InitializeMap();


	void TickIteration();
	void RunIterationsInstant();

	float ValueFunction(TArray<FEvoGraph> Graphs, FEvoGrid Grid);

	int32 IterationCounter = 0;

	UFUNCTION(BlueprintCallable)
	void RerunInstant();
};
