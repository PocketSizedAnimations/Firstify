// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Blutilities/RaffleBlueprintLibrary.h"
#include "Math/RandomStream.h"
#include "Firstify.h"



bool URaffleBlueprintLibrary::RaffleByPercent(uint8 WinningChance)
{
	UE_LOG(LogFirstify, Log, TEXT("URaffleBlueprintLibrary::RaffleByPercent(WinningChance=%i)"),WinningChance);
	/*cap-ability to go over 100%*/
	if (WinningChance > 100)
		WinningChance = 100;

	/*I mean....dunno why you need a raffle at this point - but you won!*/
	if (WinningChance == 100)
		return true;


	/*fill lottery up*/
	TArray<uint8> Lottery;	
	for (uint8 i = 0; i < 100; i++)
	{
		Lottery.Add(i);
	}

	/*grab some raffle tickets*/
	TArray<uint8> RaffledNumbers;
	for (uint8 i = 0; i < WinningChance; i++)
	{
		uint8 randomIndex = FMath::RandRange(0, Lottery.Num() - 1);
		RaffledNumbers.Add(Lottery[randomIndex]);
		UE_LOG(LogFirstify, Log, TEXT("\t grabbing ticket (%i)..."), Lottery[randomIndex]);

		Lottery.RemoveAt(randomIndex);		
	}

	/*pick winning number*/
	uint8 WinningNumber = FMath::RandRange(0, 99);
	UE_LOG(LogFirstify, Log, TEXT("\t winning ticket is....(%i)"), WinningNumber);
	
	if (RaffledNumbers.Contains(WinningNumber))
	{
		UE_LOG(LogFirstify, Log, TEXT("\t WINNER!"));
		return true;
	}
	else
	{
		UE_LOG(LogFirstify, Log, TEXT("\t LOSER!"));
		return false;
	}		
}

float URaffleBlueprintLibrary::ReturnChanceOfWinning(int32 PlayerScore, int32 TotalScore, int32 TicketPerPoints)
{
	/*simplify the math a little for efficiency*/
	if (TicketPerPoints > 0)
	{
		PlayerScore = PlayerScore / TicketPerPoints;
		TotalScore = TotalScore / TicketPerPoints;
	}

		
	return 0.0f;
}

APlayerController* URaffleBlueprintLibrary::RaffleByPlayerScores(TArray<FPlayerRaffle> PlayerRaffles, int32 TicketsPerPoints)
{
	return nullptr;
}
