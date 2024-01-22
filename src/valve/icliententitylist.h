#pragma once
#include "centity.h"

class IClientEntityList
{
public:
	constexpr CEntity* GetEntityFromIndex(const std::int32_t index) noexcept
	{
		return memory::Call<CEntity*>(this, 3, index);
	}
	constexpr std::int32_t GetHighestEntity() noexcept
	{
		return memory::Call<std::int32_t>(this, 6);
	}
	constexpr CEntity* GetEntityFromHandle(HANDLE index) noexcept
	{
		return memory::Call<CEntity*>(this, 4, index);
	}
};


