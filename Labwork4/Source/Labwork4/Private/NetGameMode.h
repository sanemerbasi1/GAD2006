#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetGameMode.generated.h"

UCLASS()
class ANetGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ANetGameMode();

    virtual void BeginPlay() override;

    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UFUNCTION(BlueprintCallable)
    void AvatarsOverlapped(ANetAvatar* AvatarA, ANetAvatar* AvatarB);

    UFUNCTION(BlueprintCallable)
    void GameTimer();

    UFUNCTION(BlueprintCallable)
    void EndGame();

    UFUNCTION()
    void ReassignTeams();

private:
    int TotalPlayerCount;
    int TotalGames;
    int PlayerStartIndex;

    FTimerHandle MatchTimerHandle;
    TArray<APlayerController*> AllPlayers;

    AActor* GetPlayerStart(FString Name, int Index);

    AActor* AssignTeamAndPlayerStart(AController* Player);
};
