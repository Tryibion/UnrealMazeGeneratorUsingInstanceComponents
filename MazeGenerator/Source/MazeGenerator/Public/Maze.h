// Code Developed by RealTryibion, Published 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Maze.generated.h"

UENUM()
enum class ECellPosition : uint8
{
	ECP_Normal,
	ECP_LeftSide,
	ECP_RightSide,
	ECP_TopSide,
	ECP_BottomSide,
	ECP_BottomRightCorner,
	ECP_BottomLeftCorner,
	ECP_TopRightCorner,
	ECP_TopLeftCorner,

	ECP_MAX
};

USTRUCT(BlueprintType)
struct FCellData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FTransform CellTransform;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint CellCoordinates;

	UPROPERTY(BlueprintReadOnly)
	int32 FloorInstNum;

	UPROPERTY(BlueprintReadOnly)
	bool bHasVisited;

	UPROPERTY(BlueprintReadOnly)
	ECellPosition CellPosition;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMazeConstructionCompleted);

class USceneComponent;
class UInstancedStaticMeshComponent;
class UStaticMesh;

UCLASS(BlueprintType, Blueprintable)
class MAZEGENERATOR_API AMaze : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMaze();

	// general maze variables
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties", meta = (ExposeOnSpawn="true"))
	int32 MazeWidth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties", meta = (ExposeOnSpawn="true"))
	int32 MazeHeight;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Maze|Properties")
	bool bGenerateInConstructionScript;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Maze|Properties", meta = (EditCondition="bGenerateInConstructionScript", ToolTip="Click to regenerate the maze in the construction script"))
	bool bRegenerateMazeInConstructionScript;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Seed", meta = (ExposeOnSpawn="true"))
	bool bUseCustomSeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Seed", meta = (EditCondition="!bUseCustomSeed", ToolTip="Click to generate a random seed", ExposeOnSpawn="true"))
	bool bGenerateRandomSeed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Seed", meta = (EditCondition="bUseCustomSeed", ExposeOnSpawn="true"))
	int32 Seed;

	UFUNCTION(BlueprintCallable, Category="Maze")
	FORCEINLINE int32 GetMazeSeed() { return Seed; }

	UFUNCTION(BlueprintCallable, Category="Maze")
	void SetMazeSeed(int32 SeedToSet) { Seed = SeedToSet; }

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Seed", meta = (ExposeOnSpawn="true"))
	FIntPoint MazeAlgorithmStartingCell;

	// variable for creating rooms
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Rooms", meta = (ExposeOnSpawn="true"))
	bool bCreateRooms;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Rooms", meta = (EditCondition="bCreateRooms", ExposeOnSpawn="true"))
	int32 NumberOfRooms;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Rooms", meta = (EditCondition="bCreateRooms", ExposeOnSpawn="true"))
	int32 RoomWidth;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Rooms", meta = (EditCondition="bCreateRooms", ExposeOnSpawn="true"))
	int32 RoomHeight;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|Rooms", meta = (EditCondition="bCreateRooms", ExposeOnSpawn="true"))
	int32 NumberOfRoomDoors;

	// variables for entry way
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (ExposeOnSpawn="true"))
	bool bHasEntry;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry",ExposeOnSpawn="true"))
	bool bCustomEntry;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && !bCustomEntry", ExposeOnSpawn="true"))
	bool bRandomEntry;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && bCustomEntry", ExposeOnSpawn="true"))
	int32 EntryWallNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && !bEntrySide2 && !bEntrySide3 && !bEntrySide4", ExposeOnSpawn="true"))
	bool bEntrySide1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && !bEntrySide1 && !bEntrySide3 && !bEntrySide4", ExposeOnSpawn="true"))
	bool bEntrySide2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && !bEntrySide1 && !bEntrySide2 && !bEntrySide4", ExposeOnSpawn="true"))
	bool bEntrySide3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|EntryPoint", meta = (EditCondition="bHasEntry && !bEntrySide1 && !bEntrySide2 && !bEntrySide3", ExposeOnSpawn="true"))
	bool bEntrySide4;

	// variables for exit way
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (ExposeOnSpawn="true"))
	bool bHasExit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit",ExposeOnSpawn="true"))
	bool bCustomExit;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && !bCustomExit", ExposeOnSpawn="true"))
	bool bRandomExit;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && bCustomExit", ExposeOnSpawn="true"))
	int32 ExitWallNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && !bExitSide2 && !bExitSide3 && !bExitSide4", ExposeOnSpawn="true"))
	bool bExitSide1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && !bExitSide1 && !bExitSide3 && !bExitSide4", ExposeOnSpawn="true"))
	bool bExitSide2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && !bExitSide1 && !bExitSide2 && !bExitSide4", ExposeOnSpawn="true"))
	bool bExitSide3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Properties|ExitPoint", meta = (EditCondition="bHasExit && !bExitSide1 && !bExitSide2 && !bExitSide3", ExposeOnSpawn="true"))
	bool bExitSide4;

	// variables for the meshes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals", meta = (ExposeOnSpawn="true"))
	UStaticMesh* FloorMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals", meta = (ExposeOnSpawn="true"))
	UStaticMesh* InnerWallMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals", meta = (ExposeOnSpawn="true"))
	UStaticMesh* OuterWallMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals", meta = (ExposeOnSpawn="true"))
	UStaticMesh* InnerCornerMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals", meta = (ExposeOnSpawn="true"))
	UStaticMesh* OuterCornerMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals|Additive", meta = (ExposeOnSpawn="true"))
	UStaticMesh* OuterBaseFloorMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Maze|Visuals|Additive", meta = (ExposeOnSpawn="true"))
	UStaticMesh* OuterBaseCornerMesh;

	// variables for the instance components
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Components")
	USceneComponent* CenterSceneComp;
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* FloorISMC;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* InnerWallISMC;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* OuterWallISMC;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* InnerCornerISMC;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* OuterCornerISMC;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* OuterBaseCornerISMC;
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category="Components")
	UInstancedStaticMeshComponent* OuterBaseFloorISMC;

	// variables for keeping track of the mesh size
	UPROPERTY()
	FVector FloorMeshSize;
	
	UPROPERTY()
	FVector InnerWallMeshSize;
	
	UPROPERTY()
	FVector OuterWallMeshSize;
	
	UPROPERTY()
	FVector InnerCornerMeshSize;
	
	UPROPERTY()
	FVector OuterCornerMeshSize;

	
	UPROPERTY()
	TMap<FIntPoint, FCellData> CellData;

	UPROPERTY()
	FRandomStream MazeRandomStream;

	UPROPERTY()
	FRandomStream RoomRandomStream;

	UPROPERTY()
	TArray<FVector> RoomCenters;
	
	UPROPERTY()
	TArray<FVector> DeadEnds;

	UPROPERTY()
	TArray<FTransform> RemovedRoomDoorwayTransforms;

	UPROPERTY()
	FTransform EntryWallTransform;

	UPROPERTY()
	FTransform ExitWallTransform;

	UPROPERTY(BlueprintAssignable, Category="Maze")
	FOnMazeConstructionCompleted OnMazeConstructionCompleted;

	UPROPERTY()
	int32 ConstructorCounter;

	bool bCompileFix;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Maze", meta = (ToolTip="Will return true if successful. This is 0 based."))
	bool DeleteFloorTile(int32 WidthX, int32 HeightY);

	UFUNCTION(BlueprintCallable, Category="Maze", meta = (ToolTip="Will return the cell location of the inserted width and height. This is 0 based."))
	FVector GetCellLocation(int32 WidthX, int32 HeightY, bool bWorld);

	UFUNCTION()
	void CreateFloorInstancesAndCellData();
	
	UFUNCTION()
	void CreateInnerAndOuterCornerInstances();

	UFUNCTION()
	void CreateInnerAndOuterWallInstances();

	UFUNCTION()
	void InitializeRandomStreamSeeds();

	UFUNCTION()
	void CreateRooms();

	UFUNCTION()
	void ImplementMazeAlgorithm();

	UFUNCTION()
	void CarveEntryAndExit();
	
	UFUNCTION(BlueprintCallable, Category="Maze")
	void ClearMaze();

	UFUNCTION(BlueprintCallable, Category="Maze")
	void GenerateRandomSeed();

	UFUNCTION(BlueprintCallable, Category="Maze")
	void ChangeValuesAndRegenerateMaze(int32 WidthOfMaze, int32 HeightOfMaze, bool bCustomSeed, int32 CustomSeed,
		bool bRandomSeed, FIntPoint StartingPointForMazeCalcs, bool bRoomCreation, int32 NumOfRooms, int32 WidthOfRoom,
		int32 HeightOfRoom, int32 NumOfRoomDoors, bool bCreateEntry, bool bHasCustomEntry, int32 CustomEntryWallNumber,
		bool bHasRandomEntry, bool bSideOfEntry1, bool bSideOfEntry2, bool bSideOfEntry3, bool bSideOfEntry4,
		bool bCreateExit, bool bHasCustomExit, int32 CustomExitWallNumber, bool bHasRandomExit, bool bSideOfExit1,
		bool bSideOfExit2, bool bSideOfExit3, bool bSideOfExit4,
		UStaticMesh* MeshForFloor, UStaticMesh* MeshForInnerWall,
		UStaticMesh* MeshForOuterWall, UStaticMesh* MeshForInnerCorner, UStaticMesh* MeshForOuterCorner,
		UStaticMesh* MeshForOuterBaseFloor, UStaticMesh* MeshForOuterBaseCorner);
	
	UFUNCTION(BlueprintCallable, Category="Maze")
	void RegenerateMaze();

	UFUNCTION(BlueprintCallable, Category="Maze")
	TArray<FVector> GetRoomCenters(bool bWorld);

	UFUNCTION(BlueprintCallable, Category="Maze")
	TArray<FVector> GetDeadEndCells(bool bWorld);

	UFUNCTION(BlueprintCallable, Category="Maze")
	TArray<FTransform> GetRemovedRoomDoorwayTransforms(bool bWorld);

	UFUNCTION(BlueprintCallable, Category="Maze")
	FTransform GetEntryWallTransform(bool bWorld);

	UFUNCTION(BlueprintCallable, Category="Maze")
	FTransform GetExitWallTransform(bool bWorld);
};