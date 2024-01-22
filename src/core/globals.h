#pragma once

class CEntity;
namespace globals
{
	inline CEntity* localPlayer = nullptr;

	// update the local player pointer
	void UpdateLocalPlayer() noexcept;

	extern float distanceBetweenPlayer;

	//bools
	extern bool visualBool[6];		//visuals
	extern bool settingsBool[3];	//settings
	extern bool movementBool[3];	//movemnt
	extern bool weaponBool[3];		//weapon
	extern bool miscBool[1];		//misc

	//misc
	extern int playerFOV;
	extern float wireColour[3];

	//weapons
	extern bool bFOVCirlce;
	extern bool bSilentAim;
	extern bool bTargetThreat;
	extern const char* target[];
	extern int selectedTarget;
	extern int selectedBone;
	extern int targetToBoneIndex[];
	extern float distanceAim;

	extern bool bwireRainbow;

	//esp
	extern bool bSnaplines;
	extern bool bHealthText;
	extern float espColor[3];
	extern float boneColor[3];
	extern bool bHealthBar;
	extern bool bArmourBar;
	extern bool bGlow;
	extern bool boxes;
	extern bool corners;
	extern int cornerSize;

	extern const char* drawFromItems[];
	extern int selectedDrawFrom;

	extern const char* diffStyles[];
	extern int selectedStyle;

	extern float enemyColour[3];
	extern float teamColour[3];

	//cahms
	extern float hidden[3];
	extern float visible[3];
	extern float chamsAlpha;

	extern float aimFOV;
}


