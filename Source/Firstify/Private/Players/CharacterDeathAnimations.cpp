// Copyright : 2020, Micah A. Parker & Pocket Sized Animations. All rights reserved


#include "Players/CharacterDeathAnimations.h"
#include "Math/UnrealMathUtility.h"


/* ReturnDeathAnimations() - Grabs a random animation from the appropriate group
* returns nullptr if the group or animations were not found
*
*
*/
UAnimSequenceBase* UCharacterDeathAnimations::ReturnDeathAnimation(FName GroupID)
{
	if (GroupID == "" || DeathAnimations.Num() <= 0)
		return nullptr;


	for (int32 i = 0; i < DeathAnimations.Num(); i++)
	{
		if (DeathAnimations[i].AnimationGroup == GroupID && DeathAnimations[i].Animations.Num() > 0) //make sure the Group ID matches && we have animations listed
		{
			int32 RandIndex = FMath::RandRange(0, DeathAnimations[i].Animations.Num() - 1);
			return DeathAnimations[i].Animations[RandIndex];
		}
	}
	
	return nullptr;
}
