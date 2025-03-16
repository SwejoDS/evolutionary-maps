// Fill out your copyright notice in the Description page of Project Settings.


#include "EvoVenice.h"
#include "EvaluationFunctionLibrary.h"

// Sets default values
AEvoVenice::AEvoVenice()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEvoVenice::BeginPlay()
{
	Super::BeginPlay();
	MapGen = Cast<UEvoMapGenerator>(NewObject<UObject>(this, MapGenClass));
	AssetSpawner = Cast<AAssetSpawnerVenice>(GetWorld()->SpawnActor(AssetSpawnerClass));
	
	InitializeMap();
}


void AEvoVenice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bTickMode && IterationCounter < MaximumIterations)
	{
		TickIteration();
	}
}

void AEvoVenice::InitializeMap()
{
	IterationCounter = 0;

	// Initialization
	if (MapGen)
	{
		FEvoGraph StreetGraph;
		StreetGraph = MapGen->InitGraph(Width, Height, EEvoTileTag::Street);
		StreetGraph = MapGen->AddNodes(StreetGraph, 4, { EEvoTileTag::PlayerStart }, false, false);
		StreetGraph = MapGen->AddNodes(StreetGraph, 1, { EEvoTileTag::Destination }, false, false);
		StreetGraph = MapGen->AddNodes(StreetGraph, 10, {}, true, false);
		StreetGraph = MapGen->AddEdges(StreetGraph, 10);
		EvoGraphs.Add(StreetGraph);

		FEvoGraph CanalGraph;
		CanalGraph = MapGen->InitGraph(Width, Height, EEvoTileTag::Canal);
		CanalGraph = MapGen->AddNodes(CanalGraph, 10, {}, true, false);
		CanalGraph = MapGen->AddEdges(CanalGraph, 10);
		EvoGraphs.Add(CanalGraph);
	}
	FEvoGrid Grid = MapGen->GenerateGridFromGraphs(EvoGraphs);
	MapGen->DrawGridToRenderTarget(this, Grid, RenderTargetAsset);

	if (!bTickMode)
	{
		RunIterationsInstant();
	}
}

void AEvoVenice::TickIteration()
{
	IterationCounter++;

	TArray<FEvoGraph> MutatedGraphs = MapGen->MutateGraphArray(EvoGraphs, MutationsPerIteration);
	FEvoGrid Grid = MapGen->GenerateGridFromGraphs(EvoGraphs);
	FEvoGrid MutatedGrid = MapGen->GenerateGridFromGraphs(MutatedGraphs);

	float Value = ValueFunction(EvoGraphs, Grid);
	float MutatedValue = ValueFunction(MutatedGraphs, MutatedGrid);

	if (MutatedValue >= Value)
	{
		EvoGraphs = MutatedGraphs;
		MapGen->DrawGridToRenderTarget(this, MutatedGrid, RenderTargetAsset);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Value after %d Iterations: %f"), IterationCounter, MutatedValue));
	}
	else
	{
		MapGen->DrawGridToRenderTarget(this, Grid, RenderTargetAsset);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Value %d Iterations %f"), IterationCounter, Value));
	}
}

void AEvoVenice::RunIterationsInstant()
{
	//int RunID = 1;  

	//FString FilePath = FPaths::ProjectDir() + FString::Printf(TEXT("EvolutionData_Run%d.csv"), RunID);
	//FString FileContent = "Iteration,Value\n"; // CSV Header

	int IterationsSinceLastIncrease = 0;

	for (int i = 0; i < MaximumIterations; i++)
	{
		IterationCounter++;

		TArray<FEvoGraph> MutatedGraphs = MapGen->MutateGraphArray(EvoGraphs, MutationsPerIteration);
		FEvoGrid Grid = MapGen->GenerateGridFromGraphs(EvoGraphs);
		FEvoGrid MutatedGrid = MapGen->GenerateGridFromGraphs(MutatedGraphs);

		float Value = ValueFunction(EvoGraphs, Grid);
		float MutatedValue = ValueFunction(MutatedGraphs, MutatedGrid);

		/*
		if (Value <= -100000 && MutatedValue > -100000)
		{
			UE_LOG(LogTemp, Warning, TEXT("Iteration: %d Value: %f"), IterationCounter, MutatedValue);
		}
		if (Value > -10000)
		{
			FileContent += FString::Printf(TEXT("%d,%f\n"), IterationCounter, Value);
		}
		*/
		if (MutatedValue >= Value)
		{
			IterationsSinceLastIncrease = 0;
			EvoGraphs = MutatedGraphs;
			MapGen->DrawGridToRenderTarget(this, MutatedGrid, RenderTargetAsset);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Value %d Iterations: %f"), IterationCounter, MutatedValue));
		}
		else
		{
			IterationsSinceLastIncrease += 1;
			MapGen->DrawGridToRenderTarget(this, Grid, RenderTargetAsset);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Value %d Iterations %f"), IterationCounter, Value));
		}
		/*
		if (IterationsSinceLastIncrease == 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("Iteration: %d Value: %f"), IterationCounter, MutatedValue);
		}
		*/

		//FFileHelper::SaveStringToFile(FileContent, *FilePath);
	}

	FEvoGrid Grid = MapGen->GenerateGridFromGraphs(EvoGraphs);
	UEvaluationFunctionLibrary::AnalyzeMap(EvoGraphs, Grid);
	
	FEvoAssetMap AssetMap = AssetSpawner->TranslateMap(Grid);
	int a = 6;
	AssetSpawner->SpawnMap(AssetMap);

}

float AEvoVenice::ValueFunction(TArray<FEvoGraph> Graphs, FEvoGrid Grid)
{
    float TotalValue = 0.0f;

	TotalValue += UEvaluationFunctionLibrary::TileCount(Grid, EEvoTileTag::Street, TargetStreetTiles);
	TotalValue += UEvaluationFunctionLibrary::TileCount(Grid, EEvoTileTag::Canal, TargetCanalTiles);
	TotalValue += UEvaluationFunctionLibrary::StreetCanalOverlap(Grid);
	TotalValue += UEvaluationFunctionLibrary::PlayerStartDestinationDistance(Graphs, Grid,TargetStartDestinationDistance);
	TotalValue += UEvaluationFunctionLibrary::StartToStartDistance(Graphs, Grid, TargetStartStartDistance);
    return TotalValue;
}

void AEvoVenice::RerunInstant()
{
	AssetSpawner->ClearMap();
	InitializeMap();
}



