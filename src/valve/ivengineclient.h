#pragma once
#include "../util/memory.h"

#include "cvector.h"
#include "cmatrix.h"
#include "ispatialquery.h"

struct PlayerInfo
{
	uint64_t version = 0ULL;
	union
	{
		uint64_t xuid;
		struct
		{
			uint32_t xuidLow;
			uint32_t xuidHigh;
		};
	};

	char name[128];
	int userId;
	char steamId[33];
	uint32_t friendsId;
	char friendsName[128];
	bool fakePlayer;
	bool isHLTV;
	int customFiles[4];
	uint8_t filesDownloaded;
};

class IVEngineClient
{

public:

	constexpr void GetScreenSize(std::int32_t& width, std::int32_t& height) noexcept
	{
		memory::Call<void>(this, 5, std::ref(width), std::ref(height));
	}

	bool GetPlayerInfo(int index, PlayerInfo* info)
	{
		return memory::Call<bool>(this, 8, index, info);
	}

	// returns index of local player
	constexpr std::int32_t GetLocalPlayerIndex() noexcept 
	{
		return memory::Call<std::int32_t>(this, 12);
	}

	constexpr void SetViewAngles(const CVector& viewAngles) noexcept 
	{
		memory::Call<void, const CVector&>(this, 19, viewAngles);
	}

	constexpr void GetViewAngles(const CVector& viewAngles) noexcept
	{
		memory::Call<void, const CVector&>(this, 18, viewAngles);
	}

	// returns true if in-game
	constexpr bool IsInGame() noexcept 
	{
		return memory::Call<bool>(this, 26);
	}

	constexpr const CMatrix4x4& WorldToScreenMatrix() noexcept
	{
		return memory::Call<const CMatrix4x4&>(this, 37);
	}

	constexpr ISpacialQuery* GetBSPTreeQuery() noexcept
	{
		return memory::Call<ISpacialQuery*>(this, 43);
	}

};
