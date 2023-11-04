#include "misc.h"

// access global variables
#include "../core/globals.h"

// access interfaces
#include "../core/interfaces.h"

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

void hacks::RunAimbot(CUserCmd* cmd) noexcept {

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
	}

	cmd->viewAngles = cmd->viewAngles + bestAng;
}

