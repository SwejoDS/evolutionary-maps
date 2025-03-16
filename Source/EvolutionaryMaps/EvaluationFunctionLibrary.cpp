// Fill out your copyright notice in the Description page of Project Settings.


#include "EvaluationFunctionLibrary.h"

float UEvaluationFunctionLibrary::TileCount(const FEvoGrid& Grid, EEvoTileTag Tag, int32 TargetCount)
{
	int32 Count = 0;
	for (int32 Y = 0; Y < Grid.Height; Y++)
	{
		for (int32 X = 0; X < Grid.Width; X++)
		{
			const FEvoTile& Tile = Grid.GetTileConst(X, Y);
			if (Tile.Tags.Contains(Tag))
			{
				Count++;
			}
		}
	}

	// Calculate the difference from target count
	float Difference = static_cast<float>(Count) - TargetCount;

	// For every tile above or below target count, add a penalty of 1
	float Penalty = -FMath::Abs(Difference);

	return Penalty;
}

float UEvaluationFunctionLibrary::StreetCanalOverlap(const FEvoGrid& Grid)
{
	float OverlapPenalty = 0.0f;

	// Directions for checking neighbors (Up, Down, Left, Right)
	const TArray<FIntPoint> Directions = {
		FIntPoint(0, 1),   // Down
		FIntPoint(0, -1),  // Up
		FIntPoint(1, 0),   // Right
		FIntPoint(-1, 0)   // Left
	};

	// Iterate over the grid
	for (int32 Y = 0; Y < Grid.Height; Y++)
	{
		for (int32 X = 0; X < Grid.Width; X++)
		{
			const FEvoTile& CurrentTile = Grid.GetTileConst(X, Y);

			// Check if the current tile has both Street and Canal tags
			if (CurrentTile.Tags.Contains(EEvoTileTag::Street) && CurrentTile.Tags.Contains(EEvoTileTag::Canal))
			{
				// Check all four neighboring tiles
				for (const FIntPoint& Dir : Directions)
				{
					int32 NeighborX = X + Dir.X;
					int32 NeighborY = Y + Dir.Y;

					// Ensure the neighbor is within grid bounds
					if (NeighborX >= 0 && NeighborX < Grid.Width && NeighborY >= 0 && NeighborY < Grid.Height)
					{
						const FEvoTile& NeighborTile = Grid.GetTileConst(NeighborX, NeighborY);

						// If the neighbor also contains both Street and Canal, apply penalty
						if (NeighborTile.Tags.Contains(EEvoTileTag::Street) && NeighborTile.Tags.Contains(EEvoTileTag::Canal))
						{
							OverlapPenalty -= 100.0f;
							break;  // No need to check other neighbors; apply penalty once per tile
						}
					}
				}
			}
		}
	}

	return OverlapPenalty;
}

float UEvaluationFunctionLibrary::PlayerStartDestinationDistance(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid, int IdealDistance)
{
	float Value = 0.0f;
	TArray<FIntPoint> StartPositions;
	FIntPoint Destination(-1, -1);

	// Search all graphs for PlayerStart and Destination nodes
	for (const FEvoGraph& Graph : Graphs)
	{
		for (const FEvoNode& Node : Graph.Nodes)
		{
			if (Node.AdditonalTags.Contains(EEvoTileTag::PlayerStart))
			{
				StartPositions.Add(Node.Location);
			}
			else if (Node.AdditonalTags.Contains(EEvoTileTag::Destination))
			{
				Destination = Node.Location;
			}
		}
	}

	// Ensure at least one PlayerStart and a valid Destination exist
	if (StartPositions.Num() == 0 || Destination == FIntPoint(-1, -1))
	{
		return -100000.0f; // Huge penalty if no start or destination exists
	}

	// Compute the shortest path for each PlayerStart to the Destination
	for (const FIntPoint& Start : StartPositions)
	{
		int32 Distance = FindShortestDistanceStreet(Grid, Start, Destination);

		if (Distance != -1) // If reachable
		{
			float Diff = static_cast<float>(Distance) - IdealDistance; 
			Value -= Diff * Diff; // Squared penalty
		}
		else // If not reachable
		{
			Value -= 100000.0f; // Large penalty for unreachable destinations
		}
	}

	return Value;
}



float UEvaluationFunctionLibrary::StartToStartDistance(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid, int IdealDistance)
{
	float Value = 0.0f;
	TArray<FIntPoint> StartPositions;

	// Collect all PlayerStart positions from all graphs
	for (const FEvoGraph& Graph : Graphs)
	{
		for (const FEvoNode& Node : Graph.Nodes)
		{
			if (Node.AdditonalTags.Contains(EEvoTileTag::PlayerStart))
			{
				StartPositions.Add(Node.Location);
			}
		}
	}

	// Ensure at least two start positions exist
	if (StartPositions.Num() < 2)
	{
		return 0.0f; // No penalty if there are fewer than two spawns
	}

	// Compare each unique pair (avoid redundant checks)
	for (int32 i = 0; i < StartPositions.Num() - 1; i++)
	{
		for (int32 j = i + 1; j < StartPositions.Num(); j++) // Ensure each pair is checked only once
		{
			int32 Distance = FindShortestDistanceStreet(Grid, StartPositions[i], StartPositions[j]);

			if (Distance != -1) // If reachable
			{
				float Diff = static_cast<float>(Distance) - IdealDistance;
				Value -= Diff * Diff;
			}
			else // If no path exists between these two spawns
			{
				Value -= 100000.0f; // Large penalty for unconnected spawns
			}
		}
	}

	return Value;
}

int32 UEvaluationFunctionLibrary::FindShortestDistanceStreet(const FEvoGrid& Grid, FIntPoint Start, FIntPoint End)
{
	// Directions for movement (up, down, left, right)
	const TArray<FIntPoint> Directions = {
		FIntPoint(0, 1),  // Down
		FIntPoint(0, -1), // Up
		FIntPoint(1, 0),  // Right
		FIntPoint(-1, 0)  // Left
	};

	// Queue for BFS
	TQueue<FIntPoint> Queue;

	// Map to store distances
	TMap<FIntPoint, int32> Distances;

	// Initialize BFS
	Queue.Enqueue(Start);
	Distances.Add(Start, 0);

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		// If we reached the end, return the distance
		if (Current == End)
		{
			return Distances[Current];
		}

		// Explore all 4 possible movements
		for (const FIntPoint& Direction : Directions)
		{
			FIntPoint Neighbor = Current + Direction;

			// Ensure the neighbor is within grid bounds
			if (Neighbor.X < 0 || Neighbor.X >= Grid.Width || Neighbor.Y < 0 || Neighbor.Y >= Grid.Height)
			{
				continue; // Skip out-of-bounds neighbors
			}

			// Get the tile at this position
			const FEvoTile& NeighborTile = Grid.GetTileConst(Neighbor.X, Neighbor.Y);

			// Check if it's a street and hasn't been visited
			if (NeighborTile.Tags.Contains(EEvoTileTag::Street) && !Distances.Contains(Neighbor))
			{
				// Add to queue
				Queue.Enqueue(Neighbor);

				// Store distance
				Distances.Add(Neighbor, Distances[Current] + 1);
			}
		}
	}

	return -1; // No valid path found
}

void UEvaluationFunctionLibrary::AnalyzeMap(const TArray<FEvoGraph>& Graphs, const FEvoGrid& Grid)
{
	int32 StreetCount = 0;
	int32 CanalCount = 0;
	int32 AdjacentWaterCanalPairs = 0;
	TArray<int32> StartToDestinationDistances;
	TArray<int32> StartToStartDistances;
	TArray<FIntPoint> StartPositions;
	FIntPoint Destination(-1, -1);

	// Iterate over grid to count Street & Canal tiles
	for (int32 Y = 0; Y < Grid.Height; Y++)
	{
		for (int32 X = 0; X < Grid.Width; X++)
		{
			const FEvoTile& Tile = Grid.GetTileConst(X, Y);

			if (Tile.Tags.Contains(EEvoTileTag::Street))
			{
				StreetCount++;
			}
			if (Tile.Tags.Contains(EEvoTileTag::Canal))
			{
				CanalCount++;
			}

			// Check for adjacent Water-Canal pairs
			const TArray<FIntPoint> Directions = {
				FIntPoint(0, 1),   // Down
				FIntPoint(0, -1),  // Up
				FIntPoint(1, 0),   // Right
				FIntPoint(-1, 0)   // Left
			};

			if (Tile.Tags.Contains(EEvoTileTag::Street) && Tile.Tags.Contains(EEvoTileTag::Canal))
			{
				// Check all four neighboring tiles
				for (const FIntPoint& Dir : Directions)
				{
					int32 NeighborX = X + Dir.X;
					int32 NeighborY = Y + Dir.Y;

					// Ensure within grid bounds
					if (NeighborX >= 0 && NeighborX < Grid.Width && NeighborY >= 0 && NeighborY < Grid.Height)
					{
						const FEvoTile& NeighborTile = Grid.GetTileConst(NeighborX, NeighborY);

						// Check if the neighbor ALSO has both Street and Canal tags
						if (NeighborTile.Tags.Contains(EEvoTileTag::Street) && NeighborTile.Tags.Contains(EEvoTileTag::Canal))
						{
							AdjacentWaterCanalPairs++;
						}
					}
				}
			}
		}
	}

	// Find PlayerStart and Destination positions
	for (const FEvoGraph& Graph : Graphs)
	{
		for (const FEvoNode& Node : Graph.Nodes)
		{
			if (Node.AdditonalTags.Contains(EEvoTileTag::PlayerStart))
			{
				StartPositions.Add(Node.Location);
			}
			else if (Node.AdditonalTags.Contains(EEvoTileTag::Destination))
			{
				Destination = Node.Location;
			}
		}
	}

	// Calculate distances from each Start to the Destination
	if (Destination != FIntPoint(-1, -1))
	{
		for (const FIntPoint& Start : StartPositions)
		{
			int32 Distance = FindShortestDistanceStreet(Grid, Start, Destination);
			if (Distance != -1)
			{
				StartToDestinationDistances.Add(Distance);
			}
		}
	}

	// Calculate distances between every Start pair
	if (StartPositions.Num() >= 2)
	{
		for (int32 i = 0; i < StartPositions.Num() - 1; i++)
		{
			for (int32 j = i + 1; j < StartPositions.Num(); j++)
			{
				int32 Distance = FindShortestDistanceStreet(Grid, StartPositions[i], StartPositions[j]);
				if (Distance != -1)
				{
					StartToStartDistances.Add(Distance);
				}
			}
		}
	}

	// Log the results
	UE_LOG(LogTemp, Warning, TEXT("Analysis Results:"));
	UE_LOG(LogTemp, Warning, TEXT("Total Street Tiles: %d"), StreetCount);
	UE_LOG(LogTemp, Warning, TEXT("Total Canal Tiles: %d"), CanalCount);
	UE_LOG(LogTemp, Warning, TEXT("Adjacent Water-Canal Pairs: %d"), AdjacentWaterCanalPairs);

	FString StartToDestinationStr = "Distances Start → Destination: ";
	for (int32 Distance : StartToDestinationDistances)
	{
		StartToDestinationStr += FString::Printf(TEXT("%d, "), Distance);
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StartToDestinationStr);

	FString StartToStartStr = "Distances Start ↔ Start: ";
	for (int32 Distance : StartToStartDistances)
	{
		StartToStartStr += FString::Printf(TEXT("%d, "), Distance);
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StartToStartStr);
}

