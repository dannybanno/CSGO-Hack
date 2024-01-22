#pragma once
#include "../util/memory.h"
#include "../core/netvars.h"

#include "cclientclass.h"
#include "cvector.h"
#include "cmatrix.h"
#include "ienginetrace.h"

#include <Windows.h>



class CEntity;
class IClientUnknown
{
public:
	constexpr CEntity* GetBaseEntity() noexcept
	{
		return memory::Call<CEntity*>(this, 7);
	}
};

class IClientRenderable
{
public:
	constexpr IClientUnknown* GetIClientUnknown() noexcept
	{
		return memory::Call<IClientUnknown*>(this, 0);
	}
};

class CModel;
class CEntity
{
public:

	enum EFlags : std::int32_t
	{
		FL_ONGROUND = (1 << 0),
		FL_DUCKING = (1 << 1),
		FL_ANIMDUCKING = (1 << 2),
		FL_WATERJUMP = (1 << 3),
		FL_ONTRAIN = (1 << 4),
		FL_INRAIN = (1 << 5),
		FL_FROZEN = (1 << 6),
		FL_ATCONTROLS = (1 << 7),
		FL_CLIENT = (1 << 8),
		FL_FAKECLIENT = (1 << 9),
		FL_INWATER = (1 << 10),
		FL_FLY = (1 << 11),
		FL_SWIM = (1 << 12),
		FL_CONVEYOR = (1 << 13),
		FL_NPC = (1 << 14),
		FL_GODMODE = (1 << 15),
		FL_NOTARGET = (1 << 16),
		FL_AIMTARGET = (1 << 17),
		FL_PARTIALGROUND = (1 << 18),
		FL_STATICPROP = (1 << 19),
		FL_GRAPHED = (1 << 20),
		FL_GRENADE = (1 << 21),
		FL_STEPMOVEMENT = (1 << 22),
		FL_DONTTOUCH = (1 << 23),
		FL_BASEVELOCITY = (1 << 24),
		FL_WORLDBRUSH = (1 << 25),
		FL_OBJECT = (1 << 26),
		FL_KILLME = (1 << 27),
		FL_ONFIRE = (1 << 28),
		FL_DISSOLVING = (1 << 29),
		FL_TRANSRAGDOLL = (1 << 30),
		FL_UNBLOCKABLE_BY_PLAYER = (1 << 31)
	};

	#define MAXSTUDIOBONES 128
	#define BONE_USED_BY_HITBOX 0x100

	enum EWeaponType : int
	{
		WEAPONTYPE_KNIFE = 0,
		WEAPONTYPE_PISTOL = 1,
		WEAPONTYPE_SUBMACHINEGUN = 2,
		WEAPONTYPE_RIFLE = 3,
		WEAPONTYPE_SHOTGUN = 4,
		WEAPONTYPE_SNIPER = 5,
		WEAPONTYPE_MACHINEGUN = 6,
		WEAPONTYPE_C4 = 7,
		WEAPONTYPE_PLACEHOLDER = 8,
		WEAPONTYPE_GRENADE = 9,
		WEAPONTYPE_HEALTHSHOT = 11,
		WEAPONTYPE_FISTS = 12,
		WEAPONTYPE_BREACHCHARGE = 13,
		WEAPONTYPE_BUMPMINE = 14,
		WEAPONTYPE_TABLET = 15,
		WEAPONTYPE_MELEE = 16
	};

	enum ItemDefinitionIndex : int {
		WEAPON_DEAGLE = 1,
		WEAPON_ELITE = 2,
		WEAPON_FIVESEVEN = 3,
		WEAPON_GLOCK = 4,
		WEAPON_AK47 = 7,
		WEAPON_AUG = 8,
		WEAPON_AWP = 9,
		WEAPON_FAMAS = 10,
		WEAPON_G3SG1 = 11,
		WEAPON_GALILAR = 13,
		WEAPON_M249 = 14,
		WEAPON_M4A1 = 16,
		WEAPON_MAC10 = 17,
		WEAPON_P90 = 19,
		WEAPON_UMP45 = 24,
		WEAPON_XM1014 = 25,
		WEAPON_BIZON = 26,
		WEAPON_MAG7 = 27,
		WEAPON_NEGEV = 28,
		WEAPON_SAWEDOFF = 29,
		WEAPON_TEC9 = 30,
		WEAPON_TASER = 31,
		WEAPON_HKP2000 = 32,
		WEAPON_MP7 = 33,
		WEAPON_MP9 = 34,
		WEAPON_NOVA = 35,
		WEAPON_P250 = 36,
		WEAPON_SCAR20 = 38,
		WEAPON_SG556 = 39,
		WEAPON_SSG08 = 40,
		WEAPON_KNIFE = 42,
		WEAPON_FLASHBANG = 43,
		WEAPON_HEGRENADE = 44,
		WEAPON_SMOKEGRENADE = 45,
		WEAPON_MOLOTOV = 46,
		WEAPON_DECOY = 47,
		WEAPON_INCGRENADE = 48,
		WEAPON_C4 = 49,
		WEAPON_KNIFE_T = 59,
		WEAPON_M4A1_SILENCER = 60,
		WEAPON_USP_SILENCER = 61,
		WEAPON_CZ75A = 63,
		WEAPON_REVOLVER = 64,
		WEAPON_KNIFE_BAYONET = 500,
		WEAPON_KNIFE_FLIP = 505,
		WEAPON_KNIFE_GUT = 506,
		WEAPON_KNIFE_KARAMBIT = 507,
		WEAPON_KNIFE_M9_BAYONET = 508,
		WEAPON_KNIFE_TACTICAL = 509,
		WEAPON_KNIFE_FALCHION = 512,
		WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
		WEAPON_KNIFE_BUTTERFLY = 515,
		WEAPON_KNIFE_PUSH = 516
	};

public: // netvars
	NETVAR(GetFlags, "CBasePlayer->m_fFlags", std::int32_t)
	NETVAR(armourValue, "CCSPlayer->m_ArmorValue", std::int32_t)
	NETVAR(healthValue, "CBasePlayer->m_iHealth", std::int32_t)
	NETVAR(IsScoped, "CCSPlayer->m_bIsScoped", bool)
	NETVAR(HasC4, "CCSPlayerResource->m_iPlayerC4", int)
	NETVAR(IsDefusing, "CCSPlayer->m_bIsDefusing", bool)
	NETVAR(HasGunGameImmunity, "CCSPlayer->m_bGunGameImmunity", bool)
	NETVAR(GetClip, "CBaseCombatWeapon->m_iClip1", int)
	NETVAR(lastPlace, "CBasePlayer->m_szLastPlaceName", std::string)
	NETVAR(activeWeapon, "CBaseCombatCharacter->m_hActiveWeapon", std::string)
	NETVAR(m_hMyWeapons, "CBaseCombatCharacter->m_hMyWeapons", HANDLE*)
	NETVAR(Spotted, "CBaseEntity->m_bSpotted", bool)
	NETVAR(observerMode, "CBasePlayer->m_iObserverMode", std::int32_t)
	NETVAR(viewOffset, "CBasePlayer->m_vecViewOffset[0]", CVector)
	NETVAR(thirdViewOffset, "CBasePlayer->m_thirdPersonViewAngles[0]", CVector)
	NETVAR(playerFOV, "CBasePlayer->m_iFOV", std::int32_t)
	NETVAR(shotsFired, "CCSPlayer->m_iShotsFired", std::int32_t)
	NETVAR(flashDuration, "CCSPlayer->m_flFlashDuration", std::int32_t)
	NETVAR(GetItemDefinitionIndex, "CBaseAttributableItem->m_iItemDefinitionIndex", short)
	NETVAR(GetDosh, "CCSPlayer->m_iAccount", std::int32_t)
	NETVAR(m_iPrimaryReserveAmmoCount, "CBaseCombatWeapon->m_iPrimaryReserveAmmoCount", int)
	NETVAR(GetOwner, "CBaseEntity->m_hOwnerEntity", int)
	NETVAR(m_hObserverTarget, "CBasePlayer->m_hObserverTarget", CEntity*)
	NETVAR(m_nFallbackPaintKit, "CBaseAttributableItem->m_nFallbackPaintKit", int)
	NETVAR(m_nFallbackSeed, "CBaseAttributableItem->m_nFallbackSeed", int)
	NETVAR(m_flFallbackWear, "CBaseAttributableItem->m_flFallbackWear", float)
	NETVAR(m_nFallbackStatTrak, "CBaseAttributableItem->m_nFallbackStatTrak", int)
	NETVAR(m_iEntityQuality, "CBaseAttributableItem->m_iEntityQuality", int)
	NETVAR(m_szCustomName, "CBaseAttributableItem->m_szCustomName", char*)
	NETVAR(m_iItemIDHigh, "CBaseAttributableItem->m_iItemIDHigh", int)

public: // renderable virtual functions (+0x4)
	constexpr CModel* GetModel() noexcept
	{
		return memory::Call<CModel*>(this + 0x4, 8);
	}

	constexpr bool SetupBones(CMatrix3x4* out, std::int32_t max, std::int32_t mask, float currentTime) noexcept
	{
		return memory::Call<bool>(this + 0x4, 13, out, max, mask, currentTime);
	}

public: // networkable virtual functions (+0x8)
	constexpr CClientClass* GetClientClass() noexcept
	{
		return memory::Call<CClientClass*>(this + 0x8, 2);
	}

	constexpr bool IsDormant() noexcept
	{
		return memory::Call<bool>(this + 0x8, 9);
	}

	constexpr std::int32_t GetIndex() noexcept
	{
		return memory::Call<std::int32_t>(this + 0x8, 10);
	}

public: // entity virtual functions
	constexpr const CVector& GetAbsOrigin() noexcept
	{
		return memory::Call<const CVector&>(this, 10);
	}

	int GetTeam()  
	{
		return memory::Call<int>(this, 88);
	}

	constexpr std::int32_t GetHealth() noexcept
	{
		return memory::Call<std::int32_t>(this, 122);
	}

	constexpr bool IsAlive() noexcept 
	{
		return memory::Call<bool>(this, 156);
	}

	constexpr bool IsPlayer() noexcept 
	{
		return memory::Call<bool>(this, 158);
	}

	constexpr bool IsWeapon() noexcept
	{
		return memory::Call<bool>(this, 166);
	}

	constexpr CEntity* GetActiveWeapon() noexcept
	{
		return memory::Call<CEntity*>(this, 268);
	}

	constexpr void GetEyePosition(CVector& eyePosition) noexcept
	{
		memory::Call<void>(this, 285, std::ref(eyePosition));
	}

	constexpr CEntity* GetObserverTarget() noexcept
	{
		return memory::Call<CEntity*>(this, 295);
	}

	constexpr void GetAimPunch(CVector& aimPunch) noexcept
	{
		memory::Call<void>(this, 346, std::ref(aimPunch));
	}

	constexpr int GetWeaponType() noexcept
	{
		return memory::Call<int>(this, 455);
	}

	ItemDefinitionIndex* GetItemDefinitionIndex2()
	{
		return (ItemDefinitionIndex*)((uintptr_t)this + GetItemDefinitionIndex());
	}

	constexpr CVector GetBonePosition(int boneIndex)
	{
		CMatrix3x4 BoneMatrix[MAXSTUDIOBONES];

		if (!this->SetupBones(BoneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0))
			return this->GetAbsOrigin();

		return CVector(BoneMatrix[boneIndex][0][3], BoneMatrix[boneIndex][1][3], BoneMatrix[boneIndex][2][3]);
	}

	std::string GetWeaponName2()
	{
		if (!this)
			return "";

		short id = this->GetClientClass()->classID;

		switch (id)
		{
		case CClientClass::CAK47:
			return std::string("AK-47");
		case CClientClass::CWeaponAWP:
			return std::string("AWP");
		case CClientClass::CDEagle:
			return std::string("Desert Eagle");
		case CClientClass::CWeaponElite:
			return std::string("Dual Berettas");
		case CClientClass::CWeaponFiveSeven:
			return std::string("Five-SeveN");
		case CClientClass::CWeaponGlock:
			return std::string("Glock-18");
		case CClientClass::CWeaponAug:
			return std::string("AUG");
		case CClientClass::CDecoyProjectile:
			return std::string("Decoy Projectile");
		case CClientClass::CFlashbang:
			return std::string("Flashbang");
		case CClientClass::CHEGrenade:
			return std::string("High-Explosive Grenade");
		case CClientClass::CInferno:
			return std::string("Inferno");
		case CClientClass::CIncendiaryGrenade:
			return std::string("Incendiary Grenade");
		case CClientClass::CMolotovProjectile:
			return std::string("Molotov Projectile");
		case CClientClass::CSmokeGrenade:
			return std::string("Smoke Grenade");
		case CClientClass::CSnowballProjectile:
			return std::string("Snowball Projectile");
		case CClientClass::CWeaponNOVA:
			return std::string("Nova");
		case CClientClass::CWeaponP250:
			return std::string("P250");
		case CClientClass::CWeaponSCAR20:
			return std::string("SCAR-20");
		case CClientClass::CWeaponSG556:
			return std::string("SG 553");
		case CClientClass::CWeaponSSG08:
			return std::string("SSG 08");
		case CClientClass::CKnife:
			return std::string("Knife");
		case CClientClass::CC4:
			return std::string("C4 Explosive");
		case CClientClass::CWeaponM4A1:
			return std::string("M4A1");
		case CClientClass::CWeaponUMP45:
			return std::string("UMP-45");
		case CClientClass::CWeaponXM1014:
			return std::string("XM1014");
		case CClientClass::CWeaponBizon:
			return std::string("PP-Bizon");
		case CClientClass::CWeaponMag7:
			return std::string("MAG-7");
		case CClientClass::CWeaponNegev:
			return std::string("Negev");
		case CClientClass::CWeaponSawedoff:
			return std::string("Sawed-Off");
		case CClientClass::CWeaponTec9:
			return std::string("Tec-9");
		case CClientClass::CWeaponTaser:
			return std::string("Taser");
		case CClientClass::CWeaponHKP2000:
			return std::string("P2000");
		case CClientClass::CWeaponMP7:
			return std::string("MP7");
		case CClientClass::CWeaponMP9:
			return std::string("MP9");
		case CClientClass::CWeaponUSP:
			return std::string("USP-S");
		default:
			return std::string("Unknown Weapon");
		}
		return "";
	}

	std::string GetWeaponName()
	{
		if (!this)
			return "";

		short id = this->GetItemDefinitionIndex();

		switch (id)
		{
		case WEAPON_DEAGLE:
			return std::string("Desert Eagle");
		case WEAPON_ELITE:
			return std::string("Dual Berettas");
		case WEAPON_FIVESEVEN:
			return std::string("Five-SeveN");
		case WEAPON_GLOCK:
			return std::string("Glock-18");
		case WEAPON_AK47:
			return std::string("AK-47");
		case WEAPON_AUG:
			return std::string("AUG");
		case WEAPON_AWP:
			return std::string("AWP");
		case WEAPON_FAMAS:
			return std::string("FAMAS");
		case WEAPON_G3SG1:
			return std::string("G3SG1");
		case WEAPON_GALILAR:
			return std::string("Galil");
		case WEAPON_M249:
			return std::string("M249");
		case WEAPON_M4A1:
			return std::string("M4A1");
		case WEAPON_MAC10:
			return std::string("MAC-10");
		case WEAPON_P90:
			return std::string("P90");
		case WEAPON_UMP45:
			return std::string("UMP-45");
		case WEAPON_XM1014:
			return std::string("XM1014");
		case WEAPON_BIZON:
			return std::string("PP-Bizon");
		case WEAPON_MAG7:
			return std::string("MAG-7");
		case WEAPON_NEGEV:
			return std::string("Negev");
		case WEAPON_SAWEDOFF:
			return std::string("Sawed-Off");
		case WEAPON_TEC9:
			return std::string("Tec-9");
		case WEAPON_TASER:
			return std::string("Taser");
		case WEAPON_HKP2000:
			return std::string("P2000");
		case WEAPON_MP7:
			return std::string("MP7");
		case WEAPON_MP9:
			return std::string("MP9");
		case WEAPON_NOVA:
			return std::string("Nova");
		case WEAPON_P250:
			return std::string("P250");
		case WEAPON_SCAR20:
			return std::string("SCAR-20");
		case WEAPON_SG556:
			return std::string("SG 553");
		case WEAPON_SSG08:
			return std::string("SSG 08");
		case WEAPON_KNIFE:
			return std::string("Knife");
		case WEAPON_FLASHBANG:
			return std::string("Flashbang");
		case WEAPON_HEGRENADE:
			return std::string("HE Grenade");
		case WEAPON_SMOKEGRENADE:
			return std::string("Smoke Grenade");
		case WEAPON_MOLOTOV:
			return std::string("Molotov");
		case WEAPON_DECOY:
			return std::string("Decoy");
		case WEAPON_INCGRENADE:
			return std::string("Incendiary Grenade");
		case WEAPON_C4:
			return std::string("C4");
		case WEAPON_KNIFE_T:
			return std::string("Knife");
		case WEAPON_M4A1_SILENCER:
			return std::string("M4A1-S");
		case WEAPON_USP_SILENCER:
			return std::string("USP-S");
		case WEAPON_CZ75A:
			return std::string("CZ75-Auto");
		case WEAPON_REVOLVER:
			return std::string("R8 Revolver");
		default:
			return std::string("Knife");
		}

		return "";
	}

};
