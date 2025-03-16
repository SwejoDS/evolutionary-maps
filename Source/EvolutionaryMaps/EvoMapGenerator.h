// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EvoStructs.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "EvoMapGenerator.generated.h"

/**
 * 
 */
UCLASS()
class EVOLUTIONARYMAPS_API UEvoMapGenerator : public UObject
{
	GENERATED_BODY()

public:

	FEvoGraph InitGraph(int Width, int Height, EEvoTileTag Tag);
	FEvoGraph AddNodes(FEvoGraph Graph, int Count, TArray<EEvoTileTag> Tags, bool bCanBeDeleted, bool bStaticLocation);
	FEvoGraph AddEdges(FEvoGraph Graph, int Count);

	TArray<FEvoGraph> MutateGraphArray(const TArray<FEvoGraph>& Graphs, int32 NumberOfMutations);


	FEvoGrid GenerateGridFromGraphs(const TArray<FEvoGraph>& Graphs);


	void DrawGridToRenderTarget(UObject* WorldContext, FEvoGrid Grid, UTextureRenderTarget2D* RenderTarget);
	
};
