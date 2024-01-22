#include "misc.h"
#include <math.h>

// access global variables
#include "../core/globals.h"

// access interfaces
#include "../core/interfaces.h"

#define DEG2RAD(x) ((x) * (3.14159265358979323846 / 180.0))

void hacks::RunBunnyHop(CUserCmd* cmd) noexcept
{
	if (!(globals::localPlayer->GetFlags() & CEntity::FL_ONGROUND))
		cmd->buttons &= ~CUserCmd::IN_JUMP;
}

bool hacks::TriggerBot(CUserCmd* cmd) noexcept {

	if (!globals::localPlayer || !globals::localPlayer->IsAlive())
		return false;

	CVector eyePos;
	globals::localPlayer->GetEyePosition(eyePos);

	CVector aimPunch;
	globals::localPlayer->GetAimPunch(aimPunch);

	const CVector dest = eyePos + CVector{ cmd->viewAngles + aimPunch }.ToVector() * 3001.f;

	CTrace trace;
	interfaces::engineTrace->TraceRay({ eyePos, dest }, 0x46004009, globals::localPlayer, trace);

	if (!trace.entity || !trace.entity->IsPlayer())
		return false;

	if (!trace.entity->IsAlive() || trace.entity->GetTeam() == globals::localPlayer->GetTeam())
		return false;

	cmd->buttons |= CUserCmd::IN_ATTACK;

	return false;
}

bool shoot = false;

void CorrectMovement(CVector vOldAngles, CUserCmd* cmd, float fOldForward, float fOldSidemove)
{
	//side/forward move correction
	float deltaView = cmd->viewAngles.y - vOldAngles.y;
	float f1;
	float f2;

	if (vOldAngles.y < 0.f)
		f1 = 360.0f + vOldAngles.y;
	else
		f1 = vOldAngles.y;

	if (cmd->viewAngles.y < 0.0f)
		f2 = 360.0f + cmd->viewAngles.y;
	else
		f2 = cmd->viewAngles.y;

	if (f2 < f1)
		deltaView = abs(f2 - f1);
	else
		deltaView = 360.0f - abs(f1 - f2);
	deltaView = 360.0f - deltaView;

	cmd->forwardMove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
	cmd->sideMove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
}

void hacks::RunSpinbot(CUserCmd* cmd) noexcept {

	if (!(cmd->buttons & CUserCmd::IN_ATTACK)) {
		cmd->viewAngles.y = cmd->viewAngles.y - 180.0f;
		cmd->viewAngles.x = 0.0f;
		cmd->viewAngles.z = 0.0f;
	}
}

void hacks::RunAimbot(CUserCmd* cmd) noexcept {

	//globals::localPlayer->observerMode() = 1;
	//globals::localPlayer->viewOffset().x = globals::localPlayer->thirdViewOffset().x;
	//globals::localPlayer->viewOffset().y = globals::localPlayer->thirdViewOffset().y;
	//globals::localPlayer->viewOffset().z = globals::localPlayer->thirdViewOffset().z;
	//interfaces::engine->SetViewAngles(globals::localPlayer->thirdViewOffset());

	//interfaces::engine->SetViewAngles(cmd->viewAngles + CVector(10, 360));
	//cmd->viewAngles = cmd->viewAngles + CVector(10, 180);

	if (!(cmd->buttons & CUserCmd::IN_ATTACK))
		return;

	if (globals::localPlayer->IsDefusing())
		return;

	CEntity* activeWeapon = globals::localPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return;

	const int weaponType = activeWeapon->GetWeaponType();

	switch (weaponType)
	{
	case CEntity::WEAPONTYPE_MACHINEGUN:
	case CEntity::WEAPONTYPE_RIFLE:
	case CEntity::WEAPONTYPE_SHOTGUN:
	case CEntity::WEAPONTYPE_SNIPER:
	case CEntity::WEAPONTYPE_PISTOL:
	{
		if (!activeWeapon->GetClip())
			return;

		if (weaponType == CEntity::WEAPONTYPE_SNIPER)
		{
			if (!globals::localPlayer->IsScoped())
				return;
		}

		break;

	}
	default:
		return;
	}

	CVector bestAng{ };
	float bestFOV{ globals::aimFOV };

	float closestPlayer = std::numeric_limits<float>::infinity();
	float bestThreat = std::numeric_limits<float>::infinity();

	for (int i = 1; i <= interfaces::globals->maxClients; ++i) {
		
		CEntity* player = interfaces::entityList->GetEntityFromIndex(i);

		if (!player)
			continue;

		if (player->IsDormant() || !player->IsAlive())
			continue;

		if (player->GetTeam() == globals::localPlayer->GetTeam())
			continue;

		if (player->HasGunGameImmunity())
			continue;

		float localPX = globals::localPlayer->GetAbsOrigin().x;
		float localPY = globals::localPlayer->GetAbsOrigin().y;
		float entX = player->GetAbsOrigin().x;
		float entY = player->GetAbsOrigin().y;

		int distance = static_cast<int>(sqrtf((entX - localPX) * (entX - localPX) + (entY - localPY) * (entY - localPY)) * 0.0254f);

		if (distance >= globals::distanceAim)
			continue;

		if (closestPlayer == std::numeric_limits<float>::infinity() || distance < closestPlayer)
			closestPlayer = distance;

		CMatrix3x4 bones[128];
		if (!player->SetupBones(bones, 128, 256, interfaces::globals->currentTime))
			continue;

		CVector localEyePos;
		globals::localPlayer->GetEyePosition(localEyePos);

		CVector aimPunch{ };

		switch (weaponType)
		{
		case CEntity::WEAPONTYPE_RIFLE:
		case CEntity::WEAPONTYPE_SUBMACHINEGUN:
		case CEntity::WEAPONTYPE_MACHINEGUN:
			globals::localPlayer->GetAimPunch(aimPunch);
		}

		CTrace trace;
		interfaces::engineTrace->TraceRay(
			CRay{ localEyePos, bones[globals::selectedBone].Origin() },
			MASK_SHOT,
			{ globals::localPlayer },
			trace
		);

		if (!trace.entity || trace.fraction < 0.97f)
			continue;

		CVector enemyAng
		{
			(bones[globals::selectedBone].Origin() - localEyePos).ToAngle() - (cmd->viewAngles + aimPunch)
		};

		if (const float fov = std::hypot(enemyAng.x, enemyAng.y); fov < bestFOV)
		{
			bestFOV = fov;
			bestAng = enemyAng;
		}

		float threatLevel = (distance * 0.5) + (static_cast<float>(player->GetHealth()) * 0.5);

		if (threatLevel < bestThreat && globals::bTargetThreat) {
			bestThreat = threatLevel;
			bestAng = enemyAng;
		}
	}

	if (globals::bSilentAim) {
		cmd->viewAngles = cmd->viewAngles + bestAng;
	}
	else {
		interfaces::engine->SetViewAngles(cmd->viewAngles + bestAng);
	}
}

