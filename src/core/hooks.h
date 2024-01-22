#pragma once
#include "interfaces.h"
#include "../menu/gui.h"

namespace hooks
{

	// call once to emplace all hooks
	void Setup();
	void SetupGUI();

	// call once to restore all hooks
	void Destroy() noexcept;

	constexpr void* VirtualFunction(void* thisptr, size_t index) noexcept
	{
		return (*static_cast<void***>(thisptr))[index];
	}

	// bypass return address checks (thx osiris)
	using AllocKeyValuesMemoryFn = void* (__thiscall*)(void*, const std::int32_t) noexcept;
	inline AllocKeyValuesMemoryFn AllocKeyValuesMemoryOriginal;
	void* __stdcall AllocKeyValuesMemory(const std::int32_t size) noexcept;

	// example CreateMove hook
	using CreateMoveFn = bool(__thiscall*)(IClientModeShared*, float, CUserCmd*) noexcept;
	inline CreateMoveFn CreateMoveOriginal = nullptr;
	bool __stdcall CreateMove(float frameTime, CUserCmd* cmd) noexcept;

	using DoPostScreenSpaceEffectsFn = void(__thiscall*)(void*, const void*) noexcept;
	inline DoPostScreenSpaceEffectsFn DoPostScreenSpaceEffectsOriginal = nullptr;
	void __stdcall DoPostScreenSpaceEffects(const void* viewSetup) noexcept;

	using EndSceneFn = long(__thiscall*)(void*, IDirect3DDevice9*) noexcept;
	inline EndSceneFn EndSceneOriginal = nullptr;
	long __stdcall EndScene(IDirect3DDevice9* device) noexcept;

	using ResetFn = HRESULT(__thiscall*)(void*, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) noexcept;
	inline ResetFn ResetOriginal = nullptr;
	HRESULT __stdcall Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept;

	using DrawModelFn = void(__thiscall*)(
		void*,
		void*,
		const CDrawModelInfo&,
		CMatrix3x4*,
		float*,
		float*,
		const CVector&,
		const std::int32_t
	) noexcept;

	inline DrawModelFn DrawModelOriginal = nullptr;

	void _stdcall DrawModel(
		void* results,
		const CDrawModelInfo& info,
		CMatrix3x4* bones,
		float* flexWeights,
		float* flexDelayedWeights,
		const CVector& modelOrigin,
		const std::int32_t flags
	)noexcept;

	using PaintTraverseFn = void(__thiscall*)(IVPanel*, std::uintptr_t, bool, bool) noexcept;
	inline PaintTraverseFn PaintTraverseOriginal = nullptr;
	void __stdcall PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept;

	//using OverrideViewFn = void(__thiscall*)(void*, ViewSetup*) noexcept;
	//inline OverrideViewFn OverrideViewOriginal = nullptr;
	//void __stdcall OverrideView(ViewSetup* setup) noexcept;

	using FrameStageNotifyFn = void(__thiscall*)(void*, ClientFrameStage_t);
	inline FrameStageNotifyFn FrameStageNotifyOriginal = nullptr;
	void __stdcall FrameStageNotify(ClientFrameStage_t stage);
}
