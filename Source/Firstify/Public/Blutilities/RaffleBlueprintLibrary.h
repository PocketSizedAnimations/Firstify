// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RaffleBlueprintLibrary.generated.h"

USTRUCT(BlueprintType)
struct FPlayerRaffle
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Raffle")
		class APlayerController* Player;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Raffle")
		int32 Score;
};


/**
 * 
 */
UCLASS()
class FIRSTIFY_API URaffleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:


	/*returns true if you won the raffle*/
	UFUNCTION(BlueprintCallable, Category = "Raffles")
		static bool RaffleByPercent(uint8 WinningChance);
	/*returns your chance of winning based on odds*/
	UFUNCTION(BlueprintCallable, Category = "Raffles")
		static float ReturnChanceOfWinning(int32 PlayerScore, int32 TotalScore, int32 TicketPerPoints);
		/*takes a list of players and their scores - and determines a winner*/
	UFUNCTION(BlueprintCallable, Category = "Raffles")
		static class APlayerController* RaffleByPlayerScores(TArray<FPlayerRaffle> PlayerRaffles, int32 TicketsPerPoints);

	
};
