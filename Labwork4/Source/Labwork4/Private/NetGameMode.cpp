// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameMode.h"
#include "NetBaseCharacter.h"
#include "NetGameState.h"
#include "NetAvatar.h"
#include "NetPlayerState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Components/CapsuleComponent.h"

ANetGameMode::ANetGameMode()
{
    DefaultPawnClass = ANetBaseCharacter::StaticClass();
    PlayerStateClass = ANetPlayerState::StaticClass();
    GameStateClass = ANetGameState::StaticClass();
}

void ANetGameMode::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &ANetGameMode::GameTimer, 30.0f, false);
}

AActor* ANetGameMode::GetPlayerStart(FString Name, int Index)
{
   FName PSName;
   if (Index < 0) {
    PSName = *Name;
   }
   else {
    PSName = *FString::Printf(TEXT("%s%d"), *Name, Index % 4);
   } 

   for (TActorIterator<APlayerStart> It(GWorld); It; ++It)
    {
       if (APlayerStart* PS = Cast<APlayerStart>(*It))
        {
        if (PS->PlayerStartTag == PSName) return *It;
        }
    }
    return nullptr;
}

AActor* ANetGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    AActor* Start = AssignTeamAndPlayerStart(Player);
    return Start ? Start : Super::ChoosePlayerStart_Implementation(Player);
}

AActor* ANetGameMode::AssignTeamAndPlayerStart(AController* Player)
{
    AActor* Start = nullptr;
    ANetPlayerState* State = Player->GetPlayerState<ANetPlayerState>();
    if(State) {

       if (TotalGames == 0)
        {
            State->TeamID = TotalPlayerCount == 0 ? EPlayerTeam::TEAM_Blue : EPlayerTeam::TEAM_Red;
            State->PlayerIndex = TotalPlayerCount++;
            AllPlayers.Add(Cast<APlayerController>(Player));
        }

        if (State->TeamID == EPlayerTeam::TEAM_Blue) {
            Start = GetPlayerStart("Blue", -1);
        } else {
            Start = GetPlayerStart("Red", PlayerStartIndex++);
        }
    }
    return Start;
}

void ANetGameMode::AvatarsOverlapped(ANetAvatar* AvatarA, ANetAvatar* AvatarB)
{
    ANetGameState* GState = GetGameState<ANetGameState>();

    if (GState == nullptr || GState->WinningPlayer >= 0) return;

    ANetPlayerState* StateA = AvatarA ? AvatarA->GetPlayerState<ANetPlayerState>() : nullptr;
    ANetPlayerState* StateB = AvatarB ? AvatarB->GetPlayerState<ANetPlayerState>() : nullptr;    
    if (!StateA || !StateB) return;
    
    if (StateA->TeamID == StateB->TeamID) return;

    GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

    if (StateA->TeamID == EPlayerTeam::TEAM_Red) {
        GState->WinningPlayer = StateA->PlayerIndex;
    } else {
        GState->WinningPlayer = StateB->PlayerIndex;
    }

    AvatarA->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    AvatarB->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

    for (APlayerController* Player : AllPlayers) 
    {
        auto State = Player->GetPlayerState<ANetPlayerState>();

        if (State->TeamID == EPlayerTeam::TEAM_Blue) {
            State->Result = EGameResult::RESULT_Lost;
        } else {
            State->Result = EGameResult::RESULT_Won;
        }
    }

    GState->OnVictory();

    FTimerHandle EndGameTimerHandle;
    GWorld->GetTimerManager().SetTimer(EndGameTimerHandle, this, &ANetGameMode::EndGame, 2.5f, false);
}

void ANetGameMode::GameTimer()
{
    ANetGameState* GState = GetGameState<ANetGameState>();

    if (GState == nullptr || GState->WinningPlayer >= 0) return;

    for (APlayerController* Player : AllPlayers)
    {
        auto State = Player->GetPlayerState<ANetPlayerState>();
        if (State && State->TeamID == EPlayerTeam::TEAM_Blue)
        {
            GState->WinningPlayer = State->PlayerIndex;
            break; 
        }
    }

    for (APlayerController* Player : AllPlayers) 
    {
        auto State = Player->GetPlayerState<ANetPlayerState>();
        if (State == nullptr) continue;

        if (State->TeamID == EPlayerTeam::TEAM_Blue) {
            State->Result = EGameResult::RESULT_Won;
        } else {
            State->Result = EGameResult::RESULT_Lost;
        }
    }

    GState->OnVictory();

    FTimerHandle EndGameTimerHandle;
    GWorld->GetTimerManager().SetTimer(EndGameTimerHandle, this, &ANetGameMode::EndGame, 2.5f, false);
}

void ANetGameMode::EndGame()
{
    PlayerStartIndex = 0;
    TotalGames++;
    ReassignTeams();
    GetGameState<ANetGameState>()->WinningPlayer = -1;

    for (APlayerController *Player : AllPlayers)
    {
        APawn* Pawn = Player->GetPawn();
        Player->UnPossess();
        if (Pawn) Pawn->Destroy();
        Player->StartSpot.Reset();
        RestartPlayer(Player);
    }

    ANetGameState* GState = GetGameState<ANetGameState>();
    GState->TriggerRestart();

    GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &ANetGameMode::GameTimer, 30.0f, false);
}

//I wanted to create a new function to swap blue randomly with a red player when blue wins, this also swaps the red when it wins with specific red that caught the blue.  
void ANetGameMode::ReassignTeams()
{
    ANetGameState* GState = GetGameState<ANetGameState>();
    if (!GState) return;

    int WinnerIndex = GState->WinningPlayer;
    bool bBlueWon = false;

    for (APlayerController* Player : AllPlayers)
    {
        auto State = Player->GetPlayerState<ANetPlayerState>();
        if (State && State->PlayerIndex == WinnerIndex)
        {
            if (State->TeamID == EPlayerTeam::TEAM_Blue)
            {
                bBlueWon = true;
            }
            break;
        }
    }
//Upper part until now is checking if blue won.

    if (bBlueWon)
    {
        TArray<ANetPlayerState*> RedCandidates;
        for (APlayerController* Player : AllPlayers)
        {
            auto State = Player->GetPlayerState<ANetPlayerState>();
            if (State && State->TeamID == EPlayerTeam::TEAM_Red)
            {
                RedCandidates.Add(State);
            }
        }

        if (RedCandidates.Num() > 0)
        {
            int RandomIndex = FMath::RandRange(0, RedCandidates.Num() - 1);
            ANetPlayerState* ChosenRed = RedCandidates[RandomIndex];

            for (APlayerController* Player : AllPlayers)
            {
                auto State = Player->GetPlayerState<ANetPlayerState>();
                if (State)
                {
                    State->TeamID = (State == ChosenRed) ? EPlayerTeam::TEAM_Blue : EPlayerTeam::TEAM_Red;
                }
            }
        }
    }
    //Upper part untill now takes all red players and randomize them to choose the next blue player.

    else
    {
        for (APlayerController* Player : AllPlayers)
        {
            auto State = Player->GetPlayerState<ANetPlayerState>();
            if (State)
            {
                State->TeamID = (State->PlayerIndex == WinnerIndex) ? EPlayerTeam::TEAM_Blue : EPlayerTeam::TEAM_Red;
            }
        }
    }
}
//Upper part until now swaps the winner with blue. Which appears when its something else than blue winning the game which means a red won the game. 