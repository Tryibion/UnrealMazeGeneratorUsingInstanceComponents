// Code Developed By RealTryibion, Published 2021


#include "Maze.h"

#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

// Sets default values
AMaze::AMaze()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CenterSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("CenterSceneComp"));
	SetRootComponent(CenterSceneComp);

	FloorISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISMC"));
	FloorISMC->SetupAttachment(GetRootComponent());

	InnerWallISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InnerWallISMC"));
	InnerWallISMC->SetupAttachment(GetRootComponent());

	OuterWallISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OuterWallISMC"));
	OuterWallISMC->SetupAttachment(GetRootComponent());

	InnerCornerISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InnerCornerISMC"));
	InnerCornerISMC->SetupAttachment(GetRootComponent());

	OuterCornerISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OuterCornerISMC"));
	OuterCornerISMC->SetupAttachment(GetRootComponent());

	OuterBaseCornerISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OuterBaseCornerISMC"));
	OuterBaseCornerISMC->SetupAttachment(GetRootComponent());

	OuterBaseFloorISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OuterBaseFloorISMC"));
	OuterBaseFloorISMC->SetupAttachment(GetRootComponent());

	MazeHeight = 20;
	MazeWidth = 20;

	NumberOfRooms = 2;
	RoomHeight = 2;
	RoomWidth = 2;

	ConstructorCounter = 0;
	FloorMeshSize = FVector(0.f);
	InnerWallMeshSize = FVector(0.f);
	InnerCornerMeshSize = FVector(0.f);
	bGenerateRandomSeed = false;
	bUseCustomSeed = false;
	bCreateRooms = false;
	MazeAlgorithmStartingCell = FIntPoint(0,0);
	bGenerateInConstructionScript = false;
	bRegenerateMazeInConstructionScript = true;
	NumberOfRoomDoors = 1;
	bCompileFix = true;

	
}

// Called when the game starts or when spawned
void AMaze::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AMaze::CreateFloorInstancesAndCellData()
{
	FloorISMC->ClearInstances();
	CellData.Empty();
	
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("No FloorMesh Set"));
		return;
	}

	// Set mesh for floor instance
	FloorISMC->SetStaticMesh(FloorMesh);
	FloorMeshSize = FloorMesh->GetBoundingBox().GetSize();
	FTransform FloorTileTransform;
	int FloorNumInstance = 0; // Used to determine cell data and floor instance coordination
	
	for (int32 i = 0; i < MazeWidth; i++)
	{
		for (int32 j = 0; j < MazeHeight; j++)
		{
			// create instance of floor and set location and initial variables
			FloorTileTransform.SetLocation(FVector(FloorMeshSize.X * i, FloorMeshSize.Y * j, 0.f));
			FloorISMC->AddInstance(FloorTileTransform);
			FCellData TempCellData;
			TempCellData.CellTransform = FloorTileTransform;
			TempCellData.bHasVisited = false;
			TempCellData.FloorInstNum = FloorNumInstance++;
			TempCellData.CellCoordinates = FIntPoint(i , j);

			// Set cell position enum based on cell location
			if (i == 0 && j == 0)
			{
				TempCellData.CellPosition = (ECellPosition::ECP_BottomLeftCorner);
			}
			else if (i == 0 && j == (MazeHeight - 1))
			{
				TempCellData.CellPosition =(ECellPosition::ECP_BottomRightCorner);
			}
			else if (i == (MazeWidth - 1) && j == 0)
			{
				TempCellData.CellPosition =(ECellPosition::ECP_TopLeftCorner);
			}
			else if (i == (MazeWidth - 1) && j == (MazeHeight - 1))
			{
				TempCellData.CellPosition =(ECellPosition::ECP_TopRightCorner);
			}
			else if (j == 0)
			{
				TempCellData.CellPosition =(ECellPosition::ECP_LeftSide);
			}
			else if (j == (MazeHeight - 1))
			{
				TempCellData.CellPosition =(ECellPosition::ECP_RightSide);
			}
			else if (i == 0)
			{
				TempCellData.CellPosition =(ECellPosition::ECP_BottomSide);
			}
			else if (i == (MazeWidth - 1))
			{
				TempCellData.CellPosition =(ECellPosition::ECP_TopSide);
			}
			else
			{
				TempCellData.CellPosition =(ECellPosition::ECP_Normal);
			}
			
			CellData.Add(FIntPoint(i,j), TempCellData);
		}
	}
}

void AMaze::CreateInnerAndOuterCornerInstances()
{
	InnerCornerISMC->ClearInstances();
	OuterCornerISMC->ClearInstances();
	OuterBaseCornerISMC->ClearInstances();
	
	if (!InnerCornerMesh || !OuterWallMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("No OuterWallMesh or InnerCornerMesh Set"));
		return;
	}

	// set mesh for instance components
	InnerCornerISMC->SetStaticMesh(InnerCornerMesh);
	OuterCornerISMC->SetStaticMesh(OuterCornerMesh);
	InnerCornerMeshSize = InnerCornerMesh->GetBoundingBox().GetSize();
	OuterCornerMeshSize = OuterCornerMesh->GetBoundingBox().GetSize();

	if (OuterBaseCornerMesh)
	{
		OuterBaseCornerISMC->SetStaticMesh(OuterBaseCornerMesh);
	}

	FTransform CornerTileTransform;

	// loop through maze width and height, set location and add instance
	for (int32 i = 0; i < MazeWidth + 1; i++)
	{
		for (int32 j = 0; j < MazeHeight + 1; j++)
		{
			if (i == 0)
			{
				CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, (OuterCornerMeshSize.Z + FloorMeshSize.Z) / 2));
				OuterCornerISMC->AddInstance(CornerTileTransform);
				if (OuterBaseCornerMesh)
				{
					CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, 0.f));
					OuterBaseCornerISMC->AddInstance(CornerTileTransform);
				}
			}
			else if (j == 0)
			{
				CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, (OuterCornerMeshSize.Z + FloorMeshSize.Z) / 2));
				OuterCornerISMC->AddInstance(CornerTileTransform);
				if (OuterBaseCornerMesh)
				{
					CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, 0.f));
					OuterBaseCornerISMC->AddInstance(CornerTileTransform);
				}
			}
			else if (i == MazeWidth)
			{
				CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, (OuterCornerMeshSize.Z + FloorMeshSize.Z) / 2));
				OuterCornerISMC->AddInstance(CornerTileTransform);
				if (OuterBaseCornerMesh)
				{
					CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, 0.f));
					OuterBaseCornerISMC->AddInstance(CornerTileTransform);
				}
			}
			else if (j == MazeHeight)
			{
				CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, (OuterCornerMeshSize.Z + FloorMeshSize.Z) / 2));
				OuterCornerISMC->AddInstance(CornerTileTransform);
				if (OuterBaseCornerMesh)
				{
					CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, 0.f));
					OuterBaseCornerISMC->AddInstance(CornerTileTransform);
				}
			}
			else
			{
				CornerTileTransform.SetLocation(FVector((FloorMeshSize.X) * i - FloorMeshSize.X / 2, (FloorMeshSize.Y) * j - FloorMeshSize.Y / 2, (InnerCornerMeshSize.Z + FloorMeshSize.Z) / 2));
				InnerCornerISMC->AddInstance(CornerTileTransform);
			}
		}
	}
}

void AMaze::CreateInnerAndOuterWallInstances()
{
	InnerWallISMC->ClearInstances();
	OuterWallISMC->ClearInstances();
	OuterBaseFloorISMC->ClearInstances();
	
	if (!InnerWallMesh || !OuterWallMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("No InnerWallMesh or OuterWallMesh Set"));
		return;
	}
	
	InnerWallISMC->SetStaticMesh(InnerWallMesh);
	OuterWallISMC->SetStaticMesh(OuterWallMesh);
	InnerWallMeshSize = InnerWallMesh->GetBoundingBox().GetSize();
	OuterWallMeshSize = OuterWallMesh->GetBoundingBox().GetSize();
	
	if (OuterBaseFloorMesh)
	{
		OuterBaseFloorISMC->SetStaticMesh(OuterBaseFloorMesh);
		
	}
	
	FTransform WallTileTransform;
	
	// Generate Walls
	// set walls +X
	for (int32 i = 0; i < MazeWidth; i++)
	{
		for (int32 j = 0; j < MazeHeight; j++)
		{
			// set outer walls
			if (i == 0)
			{
				WallTileTransform.SetRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((-FloorMeshSize.X) / 2, (OuterWallMeshSize.X + OuterCornerMeshSize.Y) * j, (OuterWallMeshSize.Z+ FloorMeshSize.Z) / 2));
				OuterWallISMC->AddInstance(WallTileTransform);
				if (OuterBaseFloorMesh)
				{
					WallTileTransform.SetLocation(FVector((-FloorMeshSize.X) / 2, (OuterWallMeshSize.X + OuterCornerMeshSize.Y) * j, 0.f));
					OuterBaseFloorISMC->AddInstance(WallTileTransform);
				}
			}
			if (i == (MazeWidth - 1))
			{
				WallTileTransform.SetRotation(FRotator(0.f, 90.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((FloorMeshSize.X * i) + (FloorMeshSize.X) / 2, (OuterWallMeshSize.X + OuterCornerMeshSize.Y) * j, (OuterWallMeshSize.Z+ FloorMeshSize.Z) / 2));
				OuterWallISMC->AddInstance(WallTileTransform);
				if (OuterBaseFloorMesh)
				{
					WallTileTransform.SetLocation(FVector((FloorMeshSize.X * i) + (FloorMeshSize.X) / 2, (OuterWallMeshSize.X + OuterCornerMeshSize.Y) * j, 0.f));
					OuterBaseFloorISMC->AddInstance(WallTileTransform);
				}
			}
			// set inner walls
			if (i != MazeWidth - 1)
			{
				WallTileTransform.SetRotation(FRotator(0.f, 90.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((FloorMeshSize.X * i) + (FloorMeshSize.X) / 2, (InnerWallMeshSize.X + InnerCornerMeshSize.Y) * j, (InnerWallMeshSize.Z+ FloorMeshSize.Z) / 2));
				InnerWallISMC->AddInstance(WallTileTransform);
			}

			// Set Outer Walls
			if (j == 0)
			{
				WallTileTransform.SetRotation(FRotator(0.f, 0.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((OuterWallMeshSize.X + OuterCornerMeshSize.X) * i, (-FloorMeshSize.Y) / 2, (OuterWallMeshSize.Z + FloorMeshSize.Z) / 2));
				OuterWallISMC->AddInstance(WallTileTransform);
				if (OuterBaseFloorMesh)
				{
					WallTileTransform.SetLocation(FVector((OuterWallMeshSize.X + OuterCornerMeshSize.X) * i, (-FloorMeshSize.Y) / 2, 0.f));
					OuterBaseFloorISMC->AddInstance(WallTileTransform);
				}
			}
			if (j == (MazeHeight - 1))
			{
				WallTileTransform.SetRotation(FRotator(0.f, 180.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((OuterWallMeshSize.X + OuterCornerMeshSize.X) * i, (FloorMeshSize.Y * j) + (FloorMeshSize.Y) / 2, (OuterWallMeshSize.Z+ FloorMeshSize.Z) / 2));
				OuterWallISMC->AddInstance(WallTileTransform);
				if (OuterBaseFloorMesh)
				{
					WallTileTransform.SetLocation(FVector((OuterWallMeshSize.X + OuterCornerMeshSize.X) * i, (FloorMeshSize.Y * j) + (FloorMeshSize.Y) / 2, 0.f));
					OuterBaseFloorISMC->AddInstance(WallTileTransform);
				}
			}
			// Set Inner Walls
			if (j != MazeHeight - 1)
			{
				WallTileTransform.SetRotation(FRotator(0.f, 180.f, 0.f).Quaternion());
				WallTileTransform.SetLocation(FVector((InnerWallMeshSize.X + InnerCornerMeshSize.X) * i, (FloorMeshSize.Y * j) + (FloorMeshSize.Y) / 2, (InnerWallMeshSize.Z+ FloorMeshSize.Z) / 2));
				InnerWallISMC->AddInstance(WallTileTransform);
			}
		}
	}
}

void AMaze::InitializeRandomStreamSeeds()
{
	MazeRandomStream.Initialize(Seed);
	RoomRandomStream.Initialize(Seed);
}

void AMaze::CreateRooms()
{
	if (!bCreateRooms || NumberOfRooms <= 0)
	{
		return;
	}
	
	RoomCenters.Empty();
	RemovedRoomDoorwayTransforms.Empty();
	
	if (RoomHeight >= MazeHeight || RoomWidth >= MazeWidth)
	{
		UE_LOG(LogTemp, Error, TEXT("Either RoomHeight or RoomWidth is set too high"));
		return;
	}
	
	int32 XRandMin = 1;
	int32 YRandMin = 1;
	int32 XRandMax = (MazeWidth - 1) - RoomWidth;
	int32 YRandMax = (MazeHeight - 1) - RoomHeight;

	for (int32 i = 0; i < NumberOfRooms; i++)
	{
		bool bRoomsSpawned = false;
		int32 LoopCounter = 0; // keeps track of times around loop, will cancel at 20
		
		LOOP: while (!bRoomsSpawned)
		{
			LoopCounter++;
			
			int32 RoomRandY = RoomRandomStream.RandRange(YRandMin, YRandMax);
			int32 RoomRandX = RoomRandomStream.RandRange(XRandMin, XRandMax);
			FIntPoint CellCoord(RoomRandX, RoomRandY);
			
			FCellData* CellFound = CellData.Find(CellCoord);
			if (!CellFound)
			{
				continue;
			}
			
			if(((NumberOfRooms * RoomWidth * RoomHeight) > ((MazeHeight * MazeWidth) - (2 * (MazeHeight - 1) + 2 * (MazeWidth - 1)))) ||
				(LoopCounter > 20))
			{
				bRoomsSpawned = true;
			}

			TArray<FVector> RoomCellLocations;
			TArray<FCellData> TempCellData;
			FBox RoomBounds;
			//UE_LOG(LogTemp, Warning, TEXT("Cell Location: %d , %d"), CellFound->CellCoordinates.X, CellFound->CellCoordinates.Y);
			
			
			for (int32 j = CellFound->CellCoordinates.X; j < CellFound->CellCoordinates.X + RoomWidth; j++)
			{
				for (int32 k = CellFound->CellCoordinates.Y; k < CellFound->CellCoordinates.Y + RoomHeight; k++)
				{
					FCellData* TempCell = CellData.Find(FIntPoint(j,k));
					if (TempCell)
					{
						// if any of the cells found have been visited before, start process over to prevent overlap
						if (TempCell->bHasVisited)
						{
							goto LOOP; // start while loop again
						}
						TempCellData.Add(*TempCell);
						RoomCellLocations.Add(TempCell->CellTransform.GetLocation());
					}
				}
			}
			
			int32 ArrayBounds = RoomCellLocations.Num() - 1; // cap the array bounds
			FVector BoxHeight = FVector(0.f, 0.f, 2 * FloorMeshSize.Z); // Vector to add to the room bounds to add height
			RoomBounds = FBox(RoomCellLocations[0] + BoxHeight, RoomCellLocations[ArrayBounds] + BoxHeight); // create box to delete wall and corner instances

			// Mark room cells as visited
			for (auto Cell : TempCellData)
			{
				FCellData* TempCell = CellData.Find(FIntPoint(Cell.CellCoordinates.X, Cell.CellCoordinates.Y));
				if (TempCell)
				{
					TempCell->bHasVisited = true;
				}
			}

			// sort largest to smallest
			auto InnerCornerToRemoveIndexes = InnerCornerISMC->GetInstancesOverlappingBox(RoomBounds, false);
			for (int32 j = 0; j < InnerCornerToRemoveIndexes.Num() - 1; j++)
			{
				for (int32 k = 0; k < InnerCornerToRemoveIndexes.Num() - j - 1; k++)
				{
					if (InnerCornerToRemoveIndexes[k] < InnerCornerToRemoveIndexes[k + 1])
					{
						InnerCornerToRemoveIndexes.Swap(k, k+1);
					}
				}
			}

			auto InnerWallToRemoveIndexes = InnerWallISMC->GetInstancesOverlappingBox(RoomBounds, false);
			for (int32 j = 0; j < InnerWallToRemoveIndexes.Num() - 1; j++)
			{
				for (int32 k = 0; k < InnerWallToRemoveIndexes.Num() - j - 1; k++)
				{
					if (InnerWallToRemoveIndexes[k] < InnerWallToRemoveIndexes[k + 1])
					{
						InnerWallToRemoveIndexes.Swap(k, k+1);
					}
				}
			}
			
			// delete wall and corner mesh instances
			for (int32 j = 0; j < InnerCornerToRemoveIndexes.Num(); j++)
			{
				InnerCornerISMC->RemoveInstance(InnerCornerToRemoveIndexes[j]);
			}
			
			for (int32 j = 0; j < InnerWallToRemoveIndexes.Num(); j++)
			{
				InnerWallISMC->RemoveInstance(InnerWallToRemoveIndexes[j]);
			}

			// carve room doors
			int32 RoomDoorCounter = 0;

			while(RoomDoorCounter < NumberOfRoomDoors)
			{
				int32 RandPosX = RoomRandomStream.RandRange(0,RoomWidth - 1);
				int32 RandPosY = RoomRandomStream.RandRange(0,RoomHeight - 1);
				int32 RandSide = RoomRandomStream.RandRange(0,3);
				FBox DoorBounds;
				for (int32 j = TempCellData[0].CellCoordinates.X; j <= TempCellData[TempCellData.Num() - 1].CellCoordinates.X; j++)
				{
					for (int32 k = TempCellData[0].CellCoordinates.Y; k <= TempCellData[TempCellData.Num() - 1].CellCoordinates.Y; k++)
					{
						if (RandSide == 0)
						{
							if (j == TempCellData[0].CellCoordinates.X)
							{
								if (k == TempCellData[0].CellCoordinates.Y + RandPosY)
								{
									DoorBounds = FBox(CellData.Find(FIntPoint(j - 1,k))->CellTransform.GetLocation() + BoxHeight,
										CellData.Find(FIntPoint(j,k))->CellTransform.GetLocation() + BoxHeight);
								}
							}
						}
						else if (RandSide == 1)
						{
							if (j == TempCellData[TempCellData.Num() - 1].CellCoordinates.X)
							{
								if (k == TempCellData[0].CellCoordinates.Y + RandPosY)
								{
									DoorBounds = FBox(CellData.Find(FIntPoint(j,k))->CellTransform.GetLocation() + BoxHeight,
										CellData.Find(FIntPoint(j + 1,k))->CellTransform.GetLocation() + BoxHeight);
								}
							}
						}
						else if (RandSide == 2)
						{
							if (k == TempCellData[0].CellCoordinates.Y)
							{
								if (j == TempCellData[0].CellCoordinates.X + RandPosX)
								{
									DoorBounds = FBox(CellData.Find(FIntPoint(j,k - 1))->CellTransform.GetLocation() + BoxHeight,
										CellData.Find(FIntPoint(j,k))->CellTransform.GetLocation() + BoxHeight);
								}
							}
						}
						else if (RandSide == 3)
						{
							if (k == TempCellData[TempCellData.Num() - 1].CellCoordinates.Y)
							{
								if (j == TempCellData[0].CellCoordinates.X + RandPosX)
								{
									DoorBounds = FBox(CellData.Find(FIntPoint(j,k))->CellTransform.GetLocation() + BoxHeight,
										CellData.Find(FIntPoint(j,k + 1))->CellTransform.GetLocation() + BoxHeight);
								}
							}
						}
					}	
				}

				auto WallsToRemove = InnerWallISMC->GetInstancesOverlappingBox(DoorBounds, false);
				FTransform DoorwayTransform;
				for (int32 j = 0; j < WallsToRemove.Num(); j++)
				{
					InnerWallISMC->GetInstanceTransform(WallsToRemove[j], DoorwayTransform, false);
					InnerWallISMC->RemoveInstance(WallsToRemove[j]);	
				}
				RemovedRoomDoorwayTransforms.Add(DoorwayTransform);
				RoomDoorCounter++;
			}

			RoomCenters.Add(RoomBounds.GetCenter());
			bRoomsSpawned = true;
		}
	}
}

void AMaze::ImplementMazeAlgorithm()
{
	FIntPoint StartingCell; // cell used to start the algorithm
	if (CellData.Find(MazeAlgorithmStartingCell))
	{
		if (MazeAlgorithmStartingCell.X > MazeWidth || MazeAlgorithmStartingCell.Y > MazeHeight || CellData.Find(MazeAlgorithmStartingCell)->bHasVisited)
		{
			UE_LOG(LogTemp, Warning, TEXT("Maze Algorithm Starting Point is out of bounds, changing to (0, 0)"));
			StartingCell = FIntPoint(0,0);
		}
		else
		{
			StartingCell = MazeAlgorithmStartingCell;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Maze Algorithm Starting Point is out of bounds, changing to (0, 0)"));
		StartingCell = FIntPoint(0,0);
	}

	DeadEnds.Empty(); // keep track of dead ends in case the user wants to spawn something there
	TArray<FCellData> CellQueue;
	int32 CurX = StartingCell.X;
	int32 CurY = StartingCell.Y;
	
	FCellData* CurrentCell = CellData.Find(FIntPoint(CurX, CurY));
	bool bDoneIterating = false;
	TArray<FCellData> UnVisitedNearbyCells;

	bool bCheckPrevious = false;
	bool bEndCounter = false; // used to find all of the dead ends
	
	while (!bDoneIterating)
	{
		if (CurrentCell)
		{
			if (!bCheckPrevious)
			{
				CellQueue.Add(*CurrentCell);
			}
			
			CurrentCell->bHasVisited = true;
			ECellPosition CurrentCellPosition = CurrentCell->CellPosition;
			int32 RandDir;
			
			FCellData* CheckCell = nullptr;
			switch (CurrentCellPosition)
			{
			case ECellPosition::ECP_Normal :
					
				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				
				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_LeftSide :
					
				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
					
				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_RightSide :

				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_TopSide :
					
				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_BottomSide :

				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
					
				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				
				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_BottomRightCorner :

				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_BottomLeftCorner :

				CheckCell = CellData.Find(FIntPoint(CurX + 1, CurY)); // cell up
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				
				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_TopRightCorner :

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY - 1)); // cell left
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			case ECellPosition::ECP_TopLeftCorner :

				CheckCell = CellData.Find(FIntPoint(CurX - 1, CurY)); // cell down
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}

				CheckCell = CellData.Find(FIntPoint(CurX, CurY + 1)); // cell right
				if (CheckCell)
				{
					if (!CheckCell->bHasVisited)
					{
						UnVisitedNearbyCells.Add(*CheckCell);
					}
				}
				break;
				
			default:
				break;
			}

			int32 ArrayLength = UnVisitedNearbyCells.Num();
			//UE_LOG(LogTemp, Warning, TEXT("%d"), ArrayLength);
			if (ArrayLength > 0)
			{
				bEndCounter = false;
				RandDir = MazeRandomStream.RandRange(0 , ArrayLength - 1);
				FCellData CellToUse = UnVisitedNearbyCells[RandDir];
				FCellData* NextCell = CellData.Find(FIntPoint(CellToUse.CellCoordinates.X, CellToUse.CellCoordinates.Y));
				if (NextCell)
				{
					FBox Box;
					if (NextCell->CellCoordinates.X < CurrentCell->CellCoordinates.X || NextCell->CellCoordinates.Y < CurrentCell->CellCoordinates.Y)
					{
						Box = FBox(NextCell->CellTransform.GetLocation()  + FVector(0.f, 0.f, 2*FloorMeshSize.Z),
						           CurrentCell->CellTransform.GetLocation() + FVector(0.f, 0.f, 2*FloorMeshSize.Z));
					}
					else
					{
						Box = FBox(CurrentCell->CellTransform.GetLocation() + FVector(0.f, 0.f, 2*FloorMeshSize.Z),
						           NextCell->CellTransform.GetLocation() + FVector(0.f, 0.f, 2*FloorMeshSize.Z));
					}
				
					InnerWallISMC->RemoveInstance(InnerWallISMC->GetInstancesOverlappingBox(Box, false)[0]);
					CurrentCell = NextCell;
					CurX = CurrentCell->CellCoordinates.X;
					CurY = CurrentCell->CellCoordinates.Y;
					bCheckPrevious = false;
				}
			}
			else
			{
				if (!bEndCounter)
				{
					DeadEnds.Add(CellQueue[CellQueue.Num() - 1].CellTransform.GetLocation());
					bEndCounter = true;
				}

				CellQueue.RemoveAt(CellQueue.Num() - 1); // remove last cell from the queue
				if (CellQueue.Num() > 0)
				{
					FCellData CellToUse = CellQueue[CellQueue.Num() - 1];
					FCellData* NextCell = CellData.Find(FIntPoint(CellToUse.CellCoordinates.X, CellToUse.CellCoordinates.Y));
					if (NextCell)
					{
						bCheckPrevious = true;
						CurrentCell = NextCell;
						CurX = CurrentCell->CellCoordinates.X;
						CurY = CurrentCell->CellCoordinates.Y;
					}
				}
				else
				{
					bDoneIterating = true;
				}
				
			}
			UnVisitedNearbyCells.Empty();
		}
		else
		{
			bDoneIterating = true;
		}
		
	}
}

void AMaze::CarveEntryAndExit()
{
	if (bHasEntry)
	{
		if (bEntrySide1)
		{
			bEntrySide2 = false;
			bEntrySide3 = false;
			bEntrySide4 = false;
		}
		else if (bEntrySide2)
		{
			bEntrySide1 = false;
			bEntrySide3 = false;
			bEntrySide4 = false;
		}
		else if (bEntrySide3)
		{
			bEntrySide1 = false;
			bEntrySide2 = false;
			bEntrySide4 = false;
		}
		else if (bEntrySide4)
		{
			bEntrySide1 = false;
			bEntrySide2 = false;
			bEntrySide3 = false;
		}
		
		if (!bCustomEntry)
		{
			if (bRandomEntry)
			{
				if (bEntrySide1 || bEntrySide3)
				{
					EntryWallNumber = MazeRandomStream.RandRange(0, MazeWidth - 1);
				}
				else if (bEntrySide2 || bEntrySide4)
				{
					EntryWallNumber = MazeRandomStream.RandRange(0, MazeHeight - 1);
				}
				else
				{
					EntryWallNumber = 0;
				}
			}
		}

		if ((bEntrySide1 || bEntrySide3) && EntryWallNumber > MazeWidth - 1)
		{
			UE_LOG(LogTemp, Error, TEXT("EntryWallNumber is greater than MazeWidth"));
			return;
		}

		if ((bEntrySide2 || bEntrySide4) && EntryWallNumber > MazeHeight - 1)
		{
			UE_LOG(LogTemp, Error, TEXT("EntryWallNumber is greater than MazeHeight"));
			return;
		}
		
		FBox EntryBox;
		FVector BoxAddition = FVector(0.f, 0.f, 2*FloorMeshSize.Z);
		if (bEntrySide1)
		{
			FCellData* Cell = CellData.Find(FIntPoint(EntryWallNumber, 0));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				EntryBox = FBox(CellLocation + FVector( 0.f, -(FloorMeshSize.Y / 2 + OuterWallMeshSize.Y), 0.f) + BoxAddition,
				CellLocation + BoxAddition);
			}
		}
		else if (bEntrySide2)
		{
			FCellData* Cell = CellData.Find(FIntPoint(0, EntryWallNumber));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				EntryBox = FBox(CellLocation + FVector( -(FloorMeshSize.X / 2 + OuterWallMeshSize.Y), 0.f, 0.f) + BoxAddition,
				CellLocation + BoxAddition);
			}
		}
		else if (bEntrySide3)
		{
			FCellData* Cell = CellData.Find(FIntPoint(EntryWallNumber, MazeHeight - 1));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				EntryBox = FBox(CellLocation + BoxAddition,
				CellLocation + FVector( 0.f, (FloorMeshSize.Y / 2 + OuterWallMeshSize.Y), 0.f) + BoxAddition);
			}
		}
		else if (bEntrySide4)
		{
			FCellData* Cell = CellData.Find(FIntPoint(MazeWidth - 1, EntryWallNumber));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				EntryBox = FBox(CellLocation + BoxAddition,
				CellLocation + FVector( (FloorMeshSize.X / 2 + OuterWallMeshSize.Y), 0.f, 0.f) + BoxAddition);
			}
		}

		
		auto EntryOverlappingWalls = OuterWallISMC->GetInstancesOverlappingBox(EntryBox, false);

		FTransform EntryTransform;
		for (auto Wall : EntryOverlappingWalls)
		{
			OuterWallISMC->GetInstanceTransform(Wall, EntryTransform, false);
			OuterWallISMC->RemoveInstance(Wall);
		}

		EntryWallTransform = EntryTransform;
		
	}
	
	if (bHasExit)
	{

		if (bExitSide1)
		{
			bExitSide2 = false;
			bExitSide3 = false;
			bExitSide4 = false;
		}
		else if (bExitSide2)
		{
			bExitSide1 = false;
			bExitSide3 = false;
			bExitSide4 = false;
		}
		else if (bExitSide3)
		{
			bExitSide1 = false;
			bExitSide2 = false;
			bExitSide4 = false;
		}
		else if (bEntrySide4)
		{
			bExitSide1 = false;
			bExitSide2 = false;
			bExitSide3 = false;
		}
		
		if (!bCustomExit)
		{
			if (bRandomExit)
			{
				if (bExitSide1 || bExitSide3)
				{
					ExitWallNumber = MazeRandomStream.RandRange(0, MazeWidth - 1);
				}
				else if (bExitSide2 || bExitSide4)
				{
					ExitWallNumber = MazeRandomStream.RandRange(0, MazeHeight - 1);
				}
				else
				{
					ExitWallNumber = 0;
				}
			}
		}

		if ((bExitSide1 || bExitSide3) && ExitWallNumber > MazeWidth - 1)
		{
			UE_LOG(LogTemp, Error, TEXT("ExitWallNumber is greater than MazeWidth"));
			return;
		}

		if ((bExitSide2 || bExitSide4) && ExitWallNumber > MazeHeight - 1)
		{
			UE_LOG(LogTemp, Error, TEXT("ExitWallNumber is greater than MazeHeight"));
			return;
		}
		
		FBox ExitBox;
		FVector BoxAddition = FVector(0.f, 0.f, 2*FloorMeshSize.Z);
		if (bExitSide1)
		{
			FCellData* Cell = CellData.Find(FIntPoint(ExitWallNumber, 0));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				ExitBox = FBox(CellLocation + FVector( 0.f, -(FloorMeshSize.Y / 2 + OuterWallMeshSize.Y), 0.f) + BoxAddition,
				CellLocation + BoxAddition);
			}
		}
		else if (bExitSide2)
		{
			FCellData* Cell = CellData.Find(FIntPoint(0, ExitWallNumber));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				ExitBox = FBox(CellLocation + FVector( -(FloorMeshSize.X / 2 + OuterWallMeshSize.Y), 0.f, 0.f) + BoxAddition,
				CellLocation + BoxAddition);
			}
		}
		else if (bExitSide3)
		{
			FCellData* Cell = CellData.Find(FIntPoint(ExitWallNumber, MazeHeight - 1));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				ExitBox = FBox(CellLocation + BoxAddition,
				CellLocation + FVector( 0.f, (FloorMeshSize.Y / 2 + OuterWallMeshSize.Y), 0.f) + BoxAddition);
			}
		}
		else if (bExitSide4)
		{
			FCellData* Cell = CellData.Find(FIntPoint(MazeWidth - 1, ExitWallNumber));
			if (Cell)
			{
				FVector CellLocation = Cell->CellTransform.GetLocation();
				ExitBox = FBox(CellLocation + BoxAddition,
				CellLocation + FVector( (FloorMeshSize.X / 2 + OuterWallMeshSize.Y), 0.f, 0.f) + BoxAddition);
			}
		}

		
		auto ExitOverlappingWalls = OuterWallISMC->GetInstancesOverlappingBox(ExitBox, false);

		FTransform ExitTransform;
		for (auto Wall : ExitOverlappingWalls)
		{
			OuterWallISMC->GetInstanceTransform(Wall, ExitTransform, false);
			OuterWallISMC->RemoveInstance(Wall);
		}

		ExitWallTransform = ExitTransform;
	}
	
}

void AMaze::ClearMaze()
{
    CellData.Empty();
	FloorISMC->ClearInstances();
	OuterCornerISMC->ClearInstances();
	OuterWallISMC->ClearInstances();
	InnerCornerISMC->ClearInstances();
	InnerWallISMC->ClearInstances();
	OuterBaseCornerISMC->ClearInstances();
	OuterBaseFloorISMC->ClearInstances();
}

void AMaze::GenerateRandomSeed()
{
	Seed = FMath::Rand();
}

void AMaze::ChangeValuesAndRegenerateMaze(int32 WidthOfMaze, int32 HeightOfMaze, bool bCustomSeed, int32 CustomSeed,
		bool bRandomSeed, FIntPoint StartingPointForMazeCalcs, bool bRoomCreation, int32 NumOfRooms, int32 WidthOfRoom,
		int32 HeightOfRoom, int32 NumOfRoomDoors, bool bCreateEntry, bool bHasCustomEntry, int32 CustomEntryWallNumber,
		bool bHasRandomEntry, bool bSideOfEntry1, bool bSideOfEntry2, bool bSideOfEntry3, bool bSideOfEntry4,
		bool bCreateExit, bool bHasCustomExit, int32 CustomExitWallNumber, bool bHasRandomExit, bool bSideOfExit1,
		bool bSideOfExit2, bool bSideOfExit3, bool bSideOfExit4,
		UStaticMesh* MeshForFloor, UStaticMesh* MeshForInnerWall,
		UStaticMesh* MeshForOuterWall, UStaticMesh* MeshForInnerCorner, UStaticMesh* MeshForOuterCorner,
		UStaticMesh* MeshForOuterBaseFloor, UStaticMesh* MeshForOuterBaseCorner)
{
	MazeWidth = WidthOfMaze;
	MazeHeight = HeightOfMaze;
	bUseCustomSeed = bCustomSeed;
	
	if (bCustomSeed)
	{
		Seed = CustomSeed;
	}
	
	bGenerateRandomSeed = bRandomSeed;
	MazeAlgorithmStartingCell = StartingPointForMazeCalcs;
	bCreateRooms = bRoomCreation;
	NumberOfRooms = NumOfRooms;
	RoomWidth = WidthOfRoom;
	RoomHeight = HeightOfRoom;
	NumberOfRoomDoors = NumOfRoomDoors;

	bHasEntry = bCreateEntry;
	bCustomEntry = bHasCustomEntry;
	EntryWallNumber = CustomEntryWallNumber;
	bRandomEntry = bHasRandomEntry;
	bEntrySide1 = bSideOfEntry1;
	bEntrySide2 = bSideOfEntry2;
	bEntrySide3 = bSideOfEntry3;
	bEntrySide4 = bSideOfEntry4;
	
	bHasExit = bCreateExit;
	bCustomExit = bHasCustomExit;
	ExitWallNumber = CustomExitWallNumber;
	bRandomExit = bHasRandomExit;
	bExitSide1 = bSideOfExit1;
	bExitSide2 = bSideOfExit2;
	bExitSide3 = bSideOfExit3;
	bExitSide4 = bSideOfExit4;
	
	if (MeshForFloor)
	{
		FloorMesh = MeshForFloor;
	}

	if (MeshForInnerWall)
	{
		InnerWallMesh = MeshForInnerWall;
	}

	if (MeshForOuterWall)
	{
		OuterWallMesh = MeshForOuterWall;
	}

	if (MeshForInnerCorner)
	{
		InnerCornerMesh = MeshForInnerCorner;
	}

	if (MeshForOuterCorner)
	{
		OuterCornerMesh = MeshForOuterCorner;
	}

	if (MeshForOuterBaseFloor)
	{
		OuterBaseFloorMesh = MeshForOuterBaseFloor;
	}

	if (MeshForOuterBaseCorner)
	{
		OuterBaseCornerMesh = MeshForOuterBaseCorner;
	}

	RegenerateMaze();
}

void AMaze::RegenerateMaze()
{
	// Set random seed if generate random seed button is clicked
	if (!bUseCustomSeed)
	{
		if (bGenerateRandomSeed)
		{
			GenerateRandomSeed();
			//bGenerateRandomSeed = false;
		}
	}

	// Set floor tiles and cell data
	CreateFloorInstancesAndCellData();
	
	// set corner meshes
	CreateInnerAndOuterCornerInstances();

	// set wall tiles
	CreateInnerAndOuterWallInstances();
	// initialize seeds
	InitializeRandomStreamSeeds();
	
	// Create Rooms
	CreateRooms();

	// generate maze
	ImplementMazeAlgorithm();

	CarveEntryAndExit();

	OnMazeConstructionCompleted.Broadcast();
}

TArray<FVector> AMaze::GetRoomCenters(bool bWorld)
{
	if (bWorld)
	{
		TArray<FVector> WorldRoomCenters;
		WorldRoomCenters.Empty();
		for (auto Room : RoomCenters)
		{
			WorldRoomCenters.Add(Room + GetActorLocation());
		}
		return WorldRoomCenters;
	}
	else
	{
		return RoomCenters;
	}
}

TArray<FVector> AMaze::GetDeadEndCells(bool bWorld)
{
	if (bWorld)
	{
		TArray<FVector> WorldDeadEnds;
		WorldDeadEnds.Empty();
		for (auto End : DeadEnds)
		{
			WorldDeadEnds.Add(End + GetActorLocation());
		}
		return WorldDeadEnds;
	}
	else
	{
		return DeadEnds;
	}
}

TArray<FTransform> AMaze::GetRemovedRoomDoorwayTransforms(bool bWorld)
{
	if (bWorld)
	{
		TArray<FTransform> WorldRemovedRoomDoorwayTransforms;
		WorldRemovedRoomDoorwayTransforms.Empty();
		for (auto Doorway : RemovedRoomDoorwayTransforms)
		{
			FTransform TempTrans = Doorway;
			TempTrans.SetLocation(Doorway.GetLocation() + GetActorLocation());
			WorldRemovedRoomDoorwayTransforms.Add(TempTrans);
		}
		return WorldRemovedRoomDoorwayTransforms;
	}
	return RemovedRoomDoorwayTransforms;
}

FTransform AMaze::GetEntryWallTransform(bool bWorld)
{
	if (bWorld)
	{
		FTransform WorldEntryTransform = EntryWallTransform;
		WorldEntryTransform.SetLocation(EntryWallTransform.GetLocation() + GetActorLocation());
		return WorldEntryTransform;
	}
	else
	{
		return EntryWallTransform;
	}
}

FTransform AMaze::GetExitWallTransform(bool bWorld)
{
	if (bWorld)
	{
		FTransform WorldExitTransform = ExitWallTransform;
		WorldExitTransform.SetLocation(ExitWallTransform.GetLocation() + GetActorLocation());
		return WorldExitTransform;
	}
	else
	{
		return ExitWallTransform;
	}
}

void AMaze::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (FloorISMC && InnerCornerISMC && OuterWallISMC && InnerCornerISMC && OuterCornerISMC)
	{
		if (bGenerateInConstructionScript)
		{
			if (bRegenerateMazeInConstructionScript || bCompileFix)
			{
				RegenerateMaze();
				bCompileFix = false;
				bRegenerateMazeInConstructionScript = false;
			}
		}
		else
		{
			ClearMaze();
			bRegenerateMazeInConstructionScript = true;
		}
	}
	
	
}

// Called every frame
void AMaze::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AMaze::DeleteFloorTile(int32 WidthX, int32 HeightY)
{
	FCellData* Cell = CellData.Find(FIntPoint( WidthX, HeightY));
	if (Cell)
	{
		int32 FloorInstNum = Cell->FloorInstNum;
		FloorISMC->RemoveInstance(FloorInstNum);
		Cell->FloorInstNum = 9999;
		for (int32 i = WidthX; i < MazeWidth; i++)
		{
			for (int32 j = HeightY; j < MazeHeight; j++)
			{
				FCellData* CellToChange = CellData.Find(FIntPoint( i, j));
				if (CellToChange)
				{
					if (WidthX == i && HeightY == j)
					{
						continue;
					}
					CellToChange->FloorInstNum = (CellToChange->FloorInstNum - 1);
					//UE_LOG(LogTemp, Error, TEXT("%d , %d , %d , %s"), CellToChange->CellCoordinates.X, CellToChange->CellCoordinates.Y, CellToChange->FloorInstNum, *CellToChange->GetCellTransform().GetLocation().ToString());
				}
			}
		}
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("Cell Location %d, %d called does not exist. Returning false."), WidthX, HeightY);
	return false;
}

FVector AMaze::GetCellLocation(int32 WidthX, int32 HeightY, bool bWorld)
{
	
	FCellData* Cell = CellData.Find(FIntPoint(WidthX, HeightY));
	if (Cell)
	{
		if (bWorld)
		{
			return Cell->CellTransform.GetLocation() + GetActorLocation() + FVector(0.f, 0.f, FloorMeshSize.Z / 2);
		}
		else
		{
			return Cell->CellTransform.GetLocation() + FVector(0.f, 0.f, FloorMeshSize.Z / 2);
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Cell Location for %d, %d called does not exist. Returning 9999."), WidthX, HeightY);
	return FVector(9999.f);
}



