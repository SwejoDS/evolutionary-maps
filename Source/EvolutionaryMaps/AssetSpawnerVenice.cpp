// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetSpawnerVenice.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AAssetSpawnerVenice::AAssetSpawnerVenice()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StreetMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StreetMeshComponent"));
	BlackBaseMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BlackBaseMeshComponent"));
	Canal_1_MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Canal_1_MeshComponent"));
	Canal_2_Straight_MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Canal_2_Straight_MeshComponent"));
	Canal_2_Curve_MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Canal_2_Curve_MeshComponent"));
	Canal_3_MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Canal_3_MeshComponent"));
	Canal_4_MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Canal_4_MeshComponent"));
	BuildingMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BuildingMeshComponent"));
	BuildingMeshComponent_02 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BuildingMeshComponent_02"));
	BuildingMeshComponent_03 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BuildingMeshComponent_03"));
	BridgeMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BridgeMeshComponent"));

	// Attach to root component if needed
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	StreetMeshComponent->SetupAttachment(RootComponent);
	BlackBaseMeshComponent->SetupAttachment(RootComponent);
	Canal_1_MeshComponent->SetupAttachment(RootComponent);
	Canal_2_Straight_MeshComponent->SetupAttachment(RootComponent);
	Canal_2_Curve_MeshComponent->SetupAttachment(RootComponent);
	Canal_3_MeshComponent->SetupAttachment(RootComponent);
	Canal_4_MeshComponent->SetupAttachment(RootComponent);
	BuildingMeshComponent->SetupAttachment(RootComponent);
	BuildingMeshComponent_02->SetupAttachment(RootComponent);
	BuildingMeshComponent_03->SetupAttachment(RootComponent);
	BridgeMeshComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AAssetSpawnerVenice::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAssetSpawnerVenice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FEvoAssetMap AAssetSpawnerVenice::TranslateMap(const FEvoGrid& Grid)
{
	int32 Width = Grid.Width;
	int32 Height = Grid.Height;
	FEvoAssetMap Map;
	Map.Initialize(Grid.Width, Grid.Height);




	// Clear any existing tile instructions
	for (FEvoTileInstruction& TileInstruction : Map.TileInstructions)
	{
		TileInstruction.Tags.Empty();
	}


	auto IsValidIndex = [&](int32 X, int32 Y) -> bool
		{
			return X >= 0 && X < Width && Y >= 0 && Y < Height;
		};

	auto HasNeighborWithTag = [&](int32 X, int32 Y, EEvoTileTag Tag) -> bool
		{
			for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
			{
				for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
				{
					if (OffsetX == 0 && OffsetY == 0)
						continue;

					int32 NeighborX = X + OffsetX;
					int32 NeighborY = Y + OffsetY;

					if (IsValidIndex(NeighborX, NeighborY))
					{
						int32 NeighborIndex = NeighborY * Width + NeighborX;
						if (Grid.Tiles[NeighborIndex].Tags.Contains(Tag))
						{
							return true;
						}
					}
				}
			}
			return false;
		};

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 Index = Y * Width + X;
			const FEvoTile& CurrentTile = Grid.GetTileConst(X,Y);
			FEvoTileInstruction& CurrentInstruction = Map.TileInstructions[Index];

			if (CurrentTile.Tags.Contains(EEvoTileTag::PlayerStart))
			{
				CurrentInstruction.Tags.Add(EEvoInstructionTag::PlayerStart);
			}

			// Determine promenade at borders
			if (CurrentTile.Tags.Contains(EEvoTileTag::Street) && !CurrentTile.Tags.Contains(EEvoTileTag::Canal))
			{
				CurrentInstruction.Tags.Add(EEvoInstructionTag::Street);
			}

			// Handle empty tiles
			if (!CurrentTile.Tags.Contains(EEvoTileTag::Street) && !CurrentTile.Tags.Contains(EEvoTileTag::Canal))
			{
				if (HasNeighborWithTag(X, Y, EEvoTileTag::Street) || HasNeighborWithTag(X, Y, EEvoTileTag::Canal))
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::Building);
				}
				CurrentInstruction.Tags.Add(EEvoInstructionTag::BlackBase);
			}

			// Handle canals
			if (CurrentTile.Tags.Contains(EEvoTileTag::Canal))
			{
				bool North = IsValidIndex(X, Y - 1) && Grid.Tiles[(Y - 1) * Width + X].Tags.Contains(EEvoTileTag::Canal);
				bool South = IsValidIndex(X, Y + 1) && Grid.Tiles[(Y + 1) * Width + X].Tags.Contains(EEvoTileTag::Canal);
				bool East = IsValidIndex(X + 1, Y) && Grid.Tiles[Y * Width + (X + 1)].Tags.Contains(EEvoTileTag::Canal);
				bool West = IsValidIndex(X - 1, Y) && Grid.Tiles[Y * Width + (X - 1)].Tags.Contains(EEvoTileTag::Canal);

				if (North && South && East && West)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalCrossroad);
				}
				else if (North && South && East)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::Canal3NoWest);
				}
				else if (North && South && West)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::Canal3NoEast);
				}
				else if (East && West && North)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::Canal3NoSouth);
				}
				else if (East && West && South)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::Canal3NoNorth);
				}
				else if (North && South)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalNorthSouth);
				}
				else if (East && West)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalEastWest);
				}
				else if (North)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalEndNorth);
				}
				else if (South)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalEndSouth);
				}
				else if (East)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalEndEast);
				}
				else if (West)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::CanalEndWest);
				}
			}

			// Handle bridges
			if (CurrentTile.Tags.Contains(EEvoTileTag::Street) && CurrentTile.Tags.Contains(EEvoTileTag::Canal))
			{
				bool VerticalCanal = IsValidIndex(X - 1, Y) && Grid.Tiles[Y * Width + (X - 1)].Tags.Contains(EEvoTileTag::Canal);
				if (!VerticalCanal)
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::BridgeNorthSouth);
				}
				else
				{
					CurrentInstruction.Tags.Add(EEvoInstructionTag::BridgeEastWest);
				}
			}
		}
	}

	return Map;
}

void AAssetSpawnerVenice::SpawnMap(FEvoAssetMap AssetMap)
{
	int32 Height = AssetMap.Height;
	int32 Width = AssetMap.Width;

	int32 NextStartAreaId = 0;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Index = Y * Width + X;
			if (!AssetMap.TileInstructions.IsValidIndex(Index))
			{
				continue;
			}
			const FEvoTileInstruction& TileInstruction = AssetMap.TileInstructions[Index];

			FVector GridCenterOffset = FVector(Width * 500.0f * 0.5f, Height * 500.0f * 0.5f, 0.0f);

			FVector InstanceLocation = FVector(X * 500.0f, Y * 500.0f, 0.0f) - GridCenterOffset;


			// SPAWN REGION

			if (TileInstruction.Tags.Contains(EEvoInstructionTag::PlayerStart))
			{
				// Spawn Player Start in the real game
			}


			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Street))
			{
				StreetMeshComponent->AddInstance(FTransform(InstanceLocation));
			}

			// Black
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::BlackBase))
			{
				BlackBaseMeshComponent->AddInstance(FTransform(InstanceLocation));
			}

			// Building
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Building))
			{
				// Pick a random building index: 0, 1, or 2
				int32 RandomBuildingIndex = FMath::RandRange(0, 2);

				// This determines the random rotation for buildings that need it
				int32 RandomRotationIndex = FMath::RandRange(0, 3);
				float YawRotation = RandomRotationIndex * 90.f;
				FRotator BuildingRotation(0.f, YawRotation, 0.f);

				switch (RandomBuildingIndex)
				{
				// Building 1
				case 0:
				{
					FTransform BuildingTransform(
						BuildingRotation,
						InstanceLocation + FVector(0.f, 0.f, 500.f)
					);
					BuildingMeshComponent->AddInstance(BuildingTransform);
					break;
				}

				// Building 2
				case 1:
				{
					FTransform BuildingTransform(
						BuildingRotation,
						InstanceLocation + FVector(0.f, 0.f, 500.f)
					);

					BuildingMeshComponent_02->AddInstance(BuildingTransform);
					break;
				}

				// Building 3
				case 2:
				{
					FTransform BuildingTransform(
						BuildingRotation,
						InstanceLocation + FVector(0.f, 0.f, 500.f)
					);

					BuildingMeshComponent_03->AddInstance(BuildingTransform);
					break;
				}
				}
			}

			// Bridge
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::BridgeNorthSouth))
			{
				FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
				BridgeMeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::BridgeEastWest))
			{
				FRotator Rotation = FRotator(0.0f, 90.0f, 0.0f);
				BridgeMeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}


			// Canal 1

			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalEndNorth))
			{
				Canal_1_MeshComponent->AddInstance(FTransform(InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalEndEast))
			{
				Canal_1_MeshComponent->AddInstance(FTransform(InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalEndSouth))
			{
				Canal_1_MeshComponent->AddInstance(FTransform(InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalEndWest))
			{
				Canal_1_MeshComponent->AddInstance(FTransform(InstanceLocation));
			}

			// Canal 2

			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalNorthSouth))
			{
				FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
				Canal_2_Straight_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalEastWest))
			{
				FRotator Rotation = FRotator(0.0f, 90.0f, 0.0f);
				Canal_2_Straight_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}

			// Canal 2 Curve


			// Canal 3
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Canal3NoNorth))
			{
				FRotator Rotation = FRotator(0.0f, 90.0f, 0.0f);
				Canal_3_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Canal3NoEast))
			{
				FRotator Rotation = FRotator(0.0f, 180.0f, 0.0f);
				Canal_3_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Canal3NoSouth))
			{
				FRotator Rotation = FRotator(0.0f, 270.0f, 0.0f);
				Canal_3_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}
			if (TileInstruction.Tags.Contains(EEvoInstructionTag::Canal3NoWest))
			{
				FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
				Canal_3_MeshComponent->AddInstance(FTransform(Rotation, InstanceLocation));
			}



			// Canal 4

			if (TileInstruction.Tags.Contains(EEvoInstructionTag::CanalCrossroad))
			{
				Canal_4_MeshComponent->AddInstance(FTransform(InstanceLocation));
			}
		}
	}
}

void AAssetSpawnerVenice::ClearMap()
{
	StreetMeshComponent->ClearInstances();
	BlackBaseMeshComponent->ClearInstances();
	Canal_1_MeshComponent->ClearInstances();
	Canal_2_Straight_MeshComponent->ClearInstances();
	Canal_2_Curve_MeshComponent->ClearInstances();
	Canal_3_MeshComponent->ClearInstances();
	Canal_4_MeshComponent->ClearInstances();
	BuildingMeshComponent->ClearInstances();
	BuildingMeshComponent_02->ClearInstances();
	BuildingMeshComponent_03->ClearInstances();
	BridgeMeshComponent->ClearInstances();
}

