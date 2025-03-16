// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EvoStructs.generated.h"

UENUM(BlueprintType)
enum class EEvoTileTag : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Street UMETA(DisplayName = "Street"),
	Canal UMETA(DisplayName = "Canal"),
	PlayerStart UMETA(DisplayName = "PlayerStart"),
	Destination UMETA(DisplayName = "Destination")
};

UENUM(BlueprintType)
enum class EEvoEdgeType : uint8
{
	HorizontalFirst,
	VerticalFirst
};

UENUM(BlueprintType)
enum class EEvoInstructionTag : uint8
{
	PlayerStart,
	PromenadeNorth,
	PromenadeSouth,
	PromenadeWest,
	PromenadeEast,
	Street,
	Building,
	BlackBase,
	CanalCrossroad,
	Canal3NoWest,
	Canal3NoEast,
	Canal3NoNorth,
	Canal3NoSouth,
	CanalNorthSouth,
	CanalEastWest,
	CanalEndNorth,
	CanalEndSouth,
	CanalEndEast,
	CanalEndWest,
	BridgeNorthSouth,
	BridgeEastWest
};


// =================================================== Graph Layer ===================================================

USTRUCT(BlueprintType)
struct FEvoNode
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EEvoTileTag> AdditonalTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanBeDeleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool StaticLocation;
};

USTRUCT(BlueprintType)
struct FEvoEdge
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FIntPoint StartNodeLocation;

	UPROPERTY()
	FIntPoint EndNodeLocation;

	UPROPERTY()
	EEvoEdgeType Type;
};

USTRUCT(BlueprintType)
struct FEvoGraph
{
	GENERATED_BODY()

public:
	UPROPERTY()
	EEvoTileTag PrimaryTileTag = EEvoTileTag::Empty;

	UPROPERTY()
	FIntPoint GridSize;

	UPROPERTY()
	TArray<FEvoNode> Nodes;

	UPROPERTY()
	TArray<FEvoEdge> Edges;

	void AddNodeAtRandomLocation(FEvoNode NewNode)
	{
		int LocX = FMath::RandRange(0, GridSize.X - 1);
		int LocY = FMath::RandRange(0, GridSize.Y - 1);
		NewNode.Location = FIntPoint(LocX, LocY);
		Nodes.Add(NewNode);
	}

	void AddRandomEdge()
	{
		if (Nodes.Num() < 2) return;

		int32 IndexA = FMath::RandRange(0, Nodes.Num() - 1);
		int32 IndexB = FMath::RandRange(0, Nodes.Num() - 1);

		if (IndexA == IndexB) return; // Ensure different nodes

		FIntPoint StartLocation = Nodes[IndexA].Location;
		FIntPoint EndLocation = Nodes[IndexB].Location;

		EEvoEdgeType RandomEdgeType = static_cast<EEvoEdgeType>(FMath::RandRange(0, 1));

		// Check if edge already exists
		bool bEdgeExists = Edges.ContainsByPredicate([StartLocation, EndLocation, RandomEdgeType](const FEvoEdge& Edge)
			{
				return (Edge.StartNodeLocation == StartLocation && Edge.EndNodeLocation == EndLocation && Edge.Type == RandomEdgeType) ||
					(Edge.StartNodeLocation == EndLocation && Edge.EndNodeLocation == StartLocation && Edge.Type == RandomEdgeType);
			});

		if (!bEdgeExists)
		{
			FEvoEdge NewEdge;
			NewEdge.StartNodeLocation = StartLocation;
			NewEdge.EndNodeLocation = EndLocation;
			NewEdge.Type = RandomEdgeType;
			Edges.Add(NewEdge);
		}
	}


	void RemoveRandomEdge()
	{
		if (Edges.Num() == 0)
		{
			return;
		}
		int32 RandomIndex = FMath::RandRange(0, Edges.Num() - 1);
		Edges.RemoveAt(RandomIndex);
	}

	void RemoveRandomNode()
	{
		if (Nodes.Num() == 0)
		{
			return;
		}

		int32 RandomIndex = FMath::RandRange(0, Nodes.Num() - 1);
		if (Nodes[RandomIndex].CanBeDeleted == false)
		{
			return;
		}
		FIntPoint NodeLocation = Nodes[RandomIndex].Location;

		Edges.RemoveAll([NodeLocation](const FEvoEdge& Edge)
			{
				return (Edge.StartNodeLocation == NodeLocation || Edge.EndNodeLocation == NodeLocation);
			});

		Nodes.RemoveAt(RandomIndex);
	}

	void MoveNode()
	{
		if (Nodes.Num() == 0)
		{
			return;
		}

		int32 RandomIndex = FMath::RandRange(0, Nodes.Num() - 1);
		FEvoNode& SelectedNode = Nodes[RandomIndex];
		FIntPoint OldLocation = SelectedNode.Location;

		FIntPoint NewLocation = FIntPoint(0,0);
		bool bLocationOccupied;

		do
		{
			NewLocation.X = FMath::RandRange(0, GridSize.X - 1);
			NewLocation.Y = FMath::RandRange(0, GridSize.Y - 1);

			bLocationOccupied = Nodes.ContainsByPredicate([NewLocation](const FEvoNode& Node)
				{
					return Node.Location == NewLocation;
				});

		} while (bLocationOccupied);

		// Update the node's location
		SelectedNode.Location = NewLocation;

		// ?? Update all edges referencing the old location
		for (FEvoEdge& Edge : Edges)
		{
			if (Edge.StartNodeLocation == OldLocation)
			{
				Edge.StartNodeLocation = NewLocation;
			}
			if (Edge.EndNodeLocation == OldLocation)
			{
				Edge.EndNodeLocation = NewLocation;
			}
		}
	}

	void ChangeEdgeMode()
	{
		if (Edges.Num() == 0)
		{
			return; // No edges to modify
		}

		int32 RandomIndex = FMath::RandRange(0, Edges.Num() - 1);

		// Toggle between edge types
		Edges[RandomIndex].Type = (Edges[RandomIndex].Type == EEvoEdgeType::HorizontalFirst)
			? EEvoEdgeType::VerticalFirst
			: EEvoEdgeType::HorizontalFirst;
	}
};

// =================================================== Grid Layer ===================================================


USTRUCT(BlueprintType)
struct FEvoTile
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FIntPoint Location;

	UPROPERTY()
	TArray<EEvoTileTag> Tags;
};

USTRUCT(BlueprintType)
struct FEvoGrid
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 Width = 64;
	
	UPROPERTY()
	int32 Height = 64;

	UPROPERTY()
	TArray<FEvoTile> Tiles;

	void Initialize(int32 NewWidth, int32 NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;

		Tiles.SetNum(Width * Height);    // Resizes the tile array
	}

	FEvoTile& GetTile(int32 X, int32 Y)
	{
		int32 Index = Y * Width + X;
		check(Tiles.IsValidIndex(Index));
		return Tiles[Index];
	}

	const FEvoTile& GetTileConst(int32 X, int32 Y) const
	{
		int32 Index = Y * Width + X;
		check(Tiles.IsValidIndex(Index));
		return Tiles[Index];
	}

	// Adds a tile tag at a specific location
	void AddTileTag(int32 X, int32 Y, EEvoTileTag Tag)
	{
		GetTile(X, Y).Tags.AddUnique(Tag);
	}
};


// =================================================== Asset Layer ===================================================

USTRUCT(BlueprintType)
struct FEvoTileInstruction
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<EEvoInstructionTag> Tags;
};

USTRUCT(BlueprintType)
struct FEvoAssetMap
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FEvoTileInstruction> TileInstructions;

	UPROPERTY()
	int Width = 64;

	UPROPERTY()
	int Height = 64;

	void Initialize(int32 NewWidth, int32 NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;
		TileInstructions.SetNum(Width * Height);
	}
};




/**
 * Empty class so this is shown within Unreal Editor
 */
UCLASS()
class EVOLUTIONARYMAPS_API UEvoStructs : public UObject
{
	GENERATED_BODY()
	
};
