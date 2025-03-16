// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EvoStructs.h"
#include "EvaluationFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class EVOLUTIONARYMAPS_API UEvaluationFunctionLibrary : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	static float TileCount(const FEvoGrid& Grid, EEvoTileTag Tag, int32 TargetCount);

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	static float StreetCanalOverlap(const FEvoGrid& Grid);

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	static float PlayerStartDestinationDistance(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid, int IdealDistance);

	UFUNCTION(BlueprintCallable, Category = "Evaluation")
	static float StartToStartDistance(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid, int IdealDistance);

	UFUNCTION(BlueprintCallable, Category = "Analysis")
	static void AnalyzeMap(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid);


private:
	static int32 FindShortestDistanceStreet(const FEvoGrid& Grid, FIntPoint Start, FIntPoint End);
};
