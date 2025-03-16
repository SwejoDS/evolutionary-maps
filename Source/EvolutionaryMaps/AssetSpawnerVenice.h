// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EvoStructs.h"
#include "AssetSpawnerVenice.generated.h"

UCLASS()
class EVOLUTIONARYMAPS_API AAssetSpawnerVenice : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAssetSpawnerVenice();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FEvoAssetMap TranslateMap(const FEvoGrid& Grid);

    void SpawnMap(FEvoAssetMap AssetMap);


    // =================================================== Mesh Instance Components =========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* StreetMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* BlackBaseMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* Canal_1_MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* Canal_2_Straight_MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* Canal_2_Curve_MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* Canal_3_MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* Canal_4_MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* BridgeMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* BuildingMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* BuildingMeshComponent_02;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UInstancedStaticMeshComponent* BuildingMeshComponent_03;

    void ClearMap();
};
