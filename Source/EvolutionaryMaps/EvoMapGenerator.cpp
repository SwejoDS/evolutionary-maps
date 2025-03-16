// Fill out your copyright notice in the Description page of Project Settings.


#include "EvoMapGenerator.h"


FEvoGraph UEvoMapGenerator::InitGraph(int Width, int Height, EEvoTileTag Tag)
{
	FEvoGraph Graph;
	Graph.GridSize = FIntPoint(Width, Height);
	Graph.PrimaryTileTag = Tag;
	return Graph;
}

FEvoGraph UEvoMapGenerator::AddNodes(FEvoGraph Graph, int Count, TArray<EEvoTileTag> Tags, bool bCanBeDeleted, bool bStaticLocation)
{
	for (int i = 0; i < Count; i++)
	{
		FEvoNode NewNode;
		NewNode.AdditonalTags = Tags;
		NewNode.CanBeDeleted = bCanBeDeleted;
		NewNode.StaticLocation = bStaticLocation;
		Graph.AddNodeAtRandomLocation(NewNode);
	}
	return Graph;
}

FEvoGraph UEvoMapGenerator::AddEdges(FEvoGraph Graph, int Count)
{
	for (int i = 0; i < Count; i++)
	{
		Graph.AddRandomEdge();
	}

	return Graph;
}

FEvoGrid UEvoMapGenerator::GenerateGridFromGraphs(const TArray<FEvoGraph>& Graphs)
{
	if (Graphs.Num() == 0)
	{
		FEvoGrid EmptyGrid;
		return EmptyGrid;
	}

	FEvoGrid Grid;
	Grid.Initialize(Graphs[0].GridSize.X, Graphs[0].GridSize.Y);

	for (const FEvoGraph& Graph : Graphs)
	{
		// Nodes
		for (const FEvoNode& Node : Graph.Nodes)
		{
			for (const EEvoTileTag Tag : Node.AdditonalTags)
			{
				Grid.AddTileTag(Node.Location.X, Node.Location.Y, Tag);
			}
		}

		for (const FEvoEdge& Edge : Graph.Edges)
		{
			FIntPoint Start = Edge.StartNodeLocation;
			FIntPoint End = Edge.EndNodeLocation;
			Start.X = FMath::Clamp(Start.X, 0, Grid.Width - 1);
			Start.Y = FMath::Clamp(Start.Y, 0, Grid.Height - 1);
			End.X = FMath::Clamp(End.X, 0, Grid.Width - 1);
			End.Y = FMath::Clamp(End.Y, 0, Grid.Height - 1);

			if (Edge.Type == EEvoEdgeType::HorizontalFirst)
			{
				// Draw horizontal segment
				int32 MinX = FMath::Min(Start.X, End.X);
				int32 MaxX = FMath::Max(Start.X, End.X);
				for (int32 X = MinX; X <= MaxX; ++X)
				{
					Grid.AddTileTag(X, Start.Y, Graph.PrimaryTileTag);
				}
				// Draw vertical segment.
				int32 MinY = FMath::Min(Start.Y, End.Y);
				int32 MaxY = FMath::Max(Start.Y, End.Y);
				for (int32 Y = MinY; Y <= MaxY; ++Y)
				{
					Grid.AddTileTag(End.X, Y, Graph.PrimaryTileTag);
				}
			}
			else
			{
				// Draw vertical segment
				int32 MinY = FMath::Min(Start.Y, End.Y);
				int32 MaxY = FMath::Max(Start.Y, End.Y);
				for (int32 Y = MinY; Y <= MaxY; ++Y)
				{
					Grid.AddTileTag(Start.X, Y, Graph.PrimaryTileTag);
				}
				// Draw horizontal segment
				int32 MinX = FMath::Min(Start.X, End.X);
				int32 MaxX = FMath::Max(Start.X, End.X);
				for (int32 X = MinX; X <= MaxX; ++X)
				{
					Grid.AddTileTag(X, End.Y, Graph.PrimaryTileTag);
				}
			}
		}
	}

	return Grid;
}


TArray<FEvoGraph> UEvoMapGenerator::MutateGraphArray(const TArray<FEvoGraph>& Graphs, int32 NumberOfMutations)
{
	TArray<FEvoGraph> MutatedGraphs = Graphs;

	if (MutatedGraphs.Num() == 0)
	{
		return MutatedGraphs;
	}
	for (int32 i = 0; i < NumberOfMutations; i++)
	{
		// Pick a random graph
		int32 RandomIndex = FMath::RandRange(0, MutatedGraphs.Num() - 1);
		FEvoGraph SelectedGraph = MutatedGraphs[RandomIndex];

		// Pick a random mutation type
		int32 MutationType = FMath::RandRange(1, 6);
		switch (MutationType)
		{
		case 1: // Remove a node (and connected edges)
			SelectedGraph.RemoveRandomNode();
			break;
		case 2: // Add a new node
		{
			FEvoNode NewNode;
			NewNode.CanBeDeleted = true;
			NewNode.StaticLocation = false;
			SelectedGraph.AddNodeAtRandomLocation(NewNode);
		}
		break;
		case 3: // Move a node (update its location and connected edges)
			SelectedGraph.MoveNode();
			break;
		case 4: // Remove an edge
			SelectedGraph.RemoveRandomEdge();
			break;
		case 5: // Add an edge
			SelectedGraph.AddRandomEdge();
			break;
		case 6: // Change an edge's mode
			SelectedGraph.ChangeEdgeMode();
			break;
		default:
			break;
		}

		// Replace the graph at RandomIndex with the mutated version
		MutatedGraphs[RandomIndex] = SelectedGraph;
	}

	return MutatedGraphs;
}


void UEvoMapGenerator::DrawGridToRenderTarget(UObject* WorldContext, FEvoGrid Grid, UTextureRenderTarget2D* RenderTarget)
{
	if (!RenderTarget) return;

	// Clear the render target with a specific color
	if (true)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(WorldContext, RenderTarget, FLinearColor::Black);
	}

	// Begin a render target drawing session
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	FCanvas Canvas(RenderTargetResource, nullptr, 0, 0, 0, WorldContext->GetWorld()->GetFeatureLevel());


	// Convert normalized positions to pixel positions and draw 1x1 squares
	int32 RT_Width = Grid.Width;
	int32 RT_Height = Grid.Height;
	FLinearColor PixelColor = FLinearColor::White;

	for (int32 Y = 0; Y < RT_Height; Y++)
	{
		for (int32 X = 0; X < RT_Width; X++)
		{
			// Calculate the index for the current tile
			int32 Index = Y * RT_Width + X;

			//if (!Grid.IsValidIndex(Index))
			//{
			//	continue; // Skip invalid indices (just in case)
			//}

			// Get the current tile
			const FEvoTile& Tile = Grid.GetTile(X, Y);

			if (Tile.Tags.Contains(EEvoTileTag::Street))
			{
				// Determine Pixel Position
				FVector2D PixelPosition(X, Y);
				FVector2D PixelSize(1.0f, 1.0f);
				PixelColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
				// Draw the pixel as a 1x1 tile
				FCanvasTileItem TileItem(PixelPosition, PixelSize, PixelColor);
				TileItem.BlendMode = SE_BLEND_Opaque;
				Canvas.DrawItem(TileItem);
			}
			else if (Tile.Tags.Contains(EEvoTileTag::Canal))
			{
				FVector2D PixelPosition(X, Y);
				FVector2D PixelSize(1.0f, 1.0f);
				PixelColor = FLinearColor::Blue;
				FCanvasTileItem TileItem(PixelPosition, PixelSize, PixelColor);
				TileItem.BlendMode = SE_BLEND_Opaque;
				Canvas.DrawItem(TileItem);
			}

			if (Tile.Tags.Contains(EEvoTileTag::PlayerStart))
			{
				FVector2D PixelPosition(X, Y);
				FVector2D PixelSize(1.0f, 1.0f);
				PixelColor = FLinearColor::Green;
				FCanvasTileItem TileItem(PixelPosition, PixelSize, PixelColor);
				TileItem.BlendMode = SE_BLEND_Opaque;
				Canvas.DrawItem(TileItem);
			}
			if (Tile.Tags.Contains(EEvoTileTag::Destination))
			{
				FVector2D PixelPosition(X, Y);
				FVector2D PixelSize(1.0f, 1.0f);
				PixelColor = FLinearColor::Red;
				FCanvasTileItem TileItem(PixelPosition, PixelSize, PixelColor);
				TileItem.BlendMode = SE_BLEND_Opaque;
				Canvas.DrawItem(TileItem);
			}
		}
	}
	// End the canvas drawing session
	Canvas.Flush_GameThread();
}