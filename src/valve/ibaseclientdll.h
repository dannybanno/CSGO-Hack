#pragma once
#include "../util/memory.h"

#include "cclientclass.h"

enum ClientFrameStage_t {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

typedef void(__thiscall *FrameStageNotify)(void*, ClientFrameStage_t);

class ViewSetup;

class IBaseClientDLL
{
public:
	constexpr CClientClass* GetAllClasses() noexcept
	{
		return memory::Call<CClientClass*>(this, 8);
	}
};
