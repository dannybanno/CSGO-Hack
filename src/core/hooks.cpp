#include "hooks.h"

// include minhook for epic hookage
#include "../../ext/minhook/minhook.h"
#include "../../ext/x86retspoof/x86RetSpoof.h"

#include <stdexcept>
#include <intrin.h>

#include "../hacks/misc.h"

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"
#include <string>
#include <iostream>


#define MAXSTUDIOBONES 128
#define BONE_USED_BY_HITBOX 0x100

int screenHeight = GetSystemMetrics(SM_CYSCREEN); int screenWidth = GetSystemMetrics(SM_CXSCREEN);

void hooks::Setup()
{
	MH_Initialize();

	// AllocKeyValuesMemory hook
	MH_CreateHook(
		memory::Get(interfaces::keyValuesSystem, 1),
		&AllocKeyValuesMemory,
		reinterpret_cast<void**>(&AllocKeyValuesMemoryOriginal)
	);

	// CreateMove hook
	MH_CreateHook(
		memory::Get(interfaces::clientMode, 24),
		&CreateMove,
		reinterpret_cast<void**>(&CreateMoveOriginal)
	);

	MH_CreateHook(
		memory::Get(interfaces::studioRender, 29),
		&DrawModel,
		reinterpret_cast<void**>(&DrawModelOriginal)
	);

	MH_CreateHook(
		memory::Get(interfaces::clientMode, 44),
		&DoPostScreenSpaceEffects,
		reinterpret_cast<void**>(&DoPostScreenSpaceEffectsOriginal)
	);

	MH_CreateHook(
		memory::Get(interfaces::panel, 41),
		&PaintTraverse,
		reinterpret_cast<void**>(&PaintTraverseOriginal)
	);

	MH_EnableHook(MH_ALL_HOOKS);
}

void hooks::SetupGUI() {
	
	if (MH_Initialize())
		throw std::runtime_error("Unable to initialize minhook");

	if (MH_CreateHook(
		VirtualFunction(gui::device, 42),
		&EndScene,
		reinterpret_cast<void**>(&EndSceneOriginal)
	)) throw std::runtime_error("Unable to hook endscene()");

	if (MH_CreateHook(
		VirtualFunction(gui::device, 16),
		&Reset,
		reinterpret_cast<void**>(&ResetOriginal)
	)) throw std::runtime_error("Unable to hook reset()");

	if (MH_EnableHook(MH_ALL_HOOKS))
		throw std::runtime_error("Unable to enable hooks");

	gui::DestroyDirectX();
}

void hooks::Destroy() noexcept
{
	// restore hooks
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	// uninit minhook
	MH_Uninitialize();
}

void* __stdcall hooks::AllocKeyValuesMemory(const std::int32_t size) noexcept
{
	// if function is returning to speficied addresses, return nullptr to "bypass"
	if (const std::uint32_t address = reinterpret_cast<std::uint32_t>(_ReturnAddress());
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesEngine) ||
		address == reinterpret_cast<std::uint32_t>(memory::allocKeyValuesClient)) 
		return nullptr;

	// return original
	return AllocKeyValuesMemoryOriginal(interfaces::keyValuesSystem, size);
}

long __stdcall hooks::EndScene(IDirect3DDevice9* device) noexcept
{
	static const auto returnAddress = _ReturnAddress();

	const auto result = EndSceneOriginal(device, device);

	//stop endscne getting called twice
	if (_ReturnAddress() == returnAddress)
		return result;

	if (!gui::setup)
		gui::SetupMenu(device);

	gui::Render();

	return result;
}

HRESULT __stdcall hooks::Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = ResetOriginal(device, device, params);
	ImGui_ImplDX9_CreateDeviceObjects();
	return result;
}

bool __stdcall hooks::CreateMove(float frameTime, CUserCmd* cmd) noexcept
{
	static const auto sequence = reinterpret_cast<std::uintptr_t>(memory::PatternScan("client.dll", "FF 23"));
	const auto result = x86RetSpoof::invokeStdcall<bool>((uintptr_t)hooks::CreateMoveOriginal, sequence, frameTime, cmd);

	// make sure this function is being called from CInput::CreateMove
	if (!cmd || !cmd->commandNumber)
		return result;

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();

	if (globals::localPlayer && globals::localPlayer->IsAlive())
	{
		if (globals::movementBool[0]) {
			hacks::RunBunnyHop(cmd);
		}
		if (globals::weaponBool[0]) {
			hacks::TriggerBot(cmd); 
		}
		if (globals::weaponBool[1]) {
			hacks::RunAimbot(cmd);
		}
	}


	return false;
}

float GetRainbowColor(float time, float frequency, float phase) {
	float r = sinf(frequency * time + 0.0f + phase) * 0.5f + 0.5f;
	float g = sinf(frequency * time + 2.0f + phase) * 0.5f + 0.5f;
	float b = sinf(frequency * time + 4.0f + phase) * 0.5f + 0.5f;

	return r, g, b;
}

void __stdcall hooks::DrawModel(
	void* results,
	const CDrawModelInfo& info,
	CMatrix3x4* bones,
	float* flexWeights,
	float* flexDelayedWeights,
	const CVector& modelOrigin,
	const std::int32_t flags
) noexcept {
	if (globals::localPlayer && info.renderable) {
		CEntity* entity = info.renderable->GetIClientUnknown()->GetBaseEntity();

		std::string pszModelName = interfaces::modelInfo->GetModelName(info.pModel);

		if (pszModelName.find("models/player")) {
			//std::cout << pszModelName;
		}

		if (globals::localPlayer && entity && !entity->IsPlayer() && globals::visualBool[3]) {
			constexpr float wireColour[3] = { 1.f, 1.f, 1.f };

			if(globals::bwireRainbow){
				static float time = 0.0f;
				time += 0.01f;
				float rainbowColor[3];
				rainbowColor[0] = GetRainbowColor(time, 0.1f, 0.0f);
				rainbowColor[1] = GetRainbowColor(time, 0.1f, 2.0f);
				rainbowColor[2] = GetRainbowColor(time, 0.1f, 4.0f);
				interfaces::studioRender->SetColorModulation(rainbowColor);
			}
			else {
				interfaces::studioRender->SetColorModulation(globals::wireColour);
			}

			static IMaterial* wireMaterial = interfaces::materialSystem->FindMaterial("debug/debugambientcube");
			wireMaterial->SetMaterialVarFlag(IMaterial::WIREFRAME, true);
			interfaces::studioRender->ForcedMaterialOverride(wireMaterial);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			return interfaces::studioRender->ForcedMaterialOverride(nullptr);
		}

		if (entity && entity->IsPlayer() && entity->GetTeam() != globals::localPlayer->GetTeam() && globals::visualBool[0]) {
			static IMaterial* material = interfaces::materialSystem->FindMaterial("debug/debugambientcube");

			interfaces::studioRender->SetAlphaModulation(globals::chamsAlpha);

			material->SetMaterialVarFlag(IMaterial::IGNOREZ, true);
			material->SetMaterialVarFlag(IMaterial::WIREFRAME, false);
			interfaces::studioRender->SetColorModulation(globals::hidden);
			interfaces::studioRender->ForcedMaterialOverride(material);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			material->SetMaterialVarFlag(IMaterial::IGNOREZ, false);
			material->SetMaterialVarFlag(IMaterial::WIREFRAME, false);
			interfaces::studioRender->SetColorModulation(globals::visible);
			interfaces::studioRender->ForcedMaterialOverride(material);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			return interfaces::studioRender->ForcedMaterialOverride(nullptr);

		}
	}
	//draw original things that armt chammed
	DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);
}


void __stdcall hooks::DoPostScreenSpaceEffects(const void* viewSetup) noexcept
{
	if (globals::localPlayer && interfaces::engine->IsInGame() && globals::visualBool[4])
	{
		for (int i = 0; i < interfaces::glow->glowObjects.size; ++i)
		{
			IGlowManager::CGlowObject& glowObject = interfaces::glow->glowObjects[i];

			if (glowObject.IsUnused())
				continue;

			if (!glowObject.entity)
				continue;

			switch (glowObject.entity->GetClientClass()->classID)
			{


			case CClientClass::CCSPlayer:
				if (!glowObject.entity->IsAlive())
					break;

				if (glowObject.entity->GetTeam() != globals::localPlayer->GetTeam())
				{
					glowObject.SetColor(globals::enemyColour[0], globals::enemyColour[1], globals::enemyColour[2]);
				}
				else {
					glowObject.SetColor(globals::teamColour[0], globals::teamColour[1], globals::teamColour[2]);
				}

				break;

			default:
				break;
			}

		}
	}

	DoPostScreenSpaceEffectsOriginal(interfaces::clientMode, viewSetup);
}

void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept
{

	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{

		if (globals::localPlayer && interfaces::engine->IsInGame())
		{

			for (int i = 1; i < interfaces::globals->maxClients; ++i)
			{
				CEntity* entity = interfaces::entityList->GetEntityFromIndex(i);

				if (!entity)
					continue;

				entity->Spotted() = true;

				if (entity->IsDormant() || !entity->IsAlive())
					continue;

				if (entity->GetTeam() == globals::localPlayer->GetTeam())
					continue;

				if (!globals::localPlayer->IsAlive())
					if (globals::localPlayer->GetObserverTarget() == entity)
						continue;

				CMatrix3x4 bones[128];
				if (!entity->SetupBones(bones, 128, 0x7FF00, interfaces::globals->currentTime))
					continue;

				CVector top;
				if (interfaces::debugOverlay->ScreenPosition(bones[8].Origin() + CVector{ 0.f, 0.f, 11.f }, top))
					continue;

				CVector bottom;
				if (interfaces::debugOverlay->ScreenPosition(entity->GetAbsOrigin() - CVector{ 0.f, 0.f, 9.f }, bottom))
					continue;

				//boxes
				const float h = bottom.y - top.y; const float w = h * 0.3f;

				const auto left = static_cast<int>(top.x - w); const auto right = static_cast<int>(top.x + w);

				int rectCenterX = (left + right) / 2;
				int rectBottomY = bottom.y;

				int health = 0;
				int armour = 0;
				health = entity->healthValue();
				armour = entity->armourValue();

				int screenCenterX = screenWidth / 2;
				int screenCenterY = screenHeight / 2;
				int screenBottomY = screenHeight;

				if (globals::visualBool[1]) {

					interfaces::surface->DrawSetColor(
						static_cast<int>(globals::espColor[0] * 255),
						static_cast<int>(globals::espColor[1] * 255),
						static_cast<int>(globals::espColor[2] * 255),
						255
					);
					if (globals::selectedStyle == 0) {
						interfaces::surface->DrawOutlinedRect(left, top.y, right, bottom.y);
					}
					if (globals::selectedStyle == 1) {
						interfaces::surface->DrawLine(left, top.y, left + globals::cornerSize, top.y);
						interfaces::surface->DrawLine(left, top.y, left, top.y + globals::cornerSize);

						interfaces::surface->DrawLine(right, top.y, right - globals::cornerSize, top.y);
						interfaces::surface->DrawLine(right, top.y, right, top.y + globals::cornerSize);

						interfaces::surface->DrawLine(left, bottom.y, left + globals::cornerSize, bottom.y);
						interfaces::surface->DrawLine(left, bottom.y, left, bottom.y - globals::cornerSize);

						interfaces::surface->DrawLine(right, bottom.y, right - globals::cornerSize, bottom.y);
						interfaces::surface->DrawLine(right, bottom.y, right, bottom.y - globals::cornerSize);
					}
					if (globals::bSnaplines) {
						if (globals::selectedDrawFrom == 0) {
							interfaces::surface->DrawLine(screenCenterX, screenBottomY, rectCenterX, rectBottomY);
						}
						else if (globals::selectedDrawFrom == 1) {
							interfaces::surface->DrawLine(screenCenterX, screenCenterY, rectCenterX, rectBottomY);
						}
					}

					interfaces::surface->DrawSetColor(31, 31, 31, 225);

					int backgroundLeft = left - 6;
					int backgroundRight = left - 3;
					int backgroundTop = top.y;
					int backgroundBottom = top.y + h;

					if (globals::bHealthBar) {
						interfaces::surface->DrawFilledRect(backgroundLeft, backgroundTop, backgroundRight, backgroundBottom);
					}


					//health abr
					int healthBarHeight = static_cast<int>(h * (health / 100.0f));

					int healthBarLeft = left - 6; int healthBarRight = left - 3;
					int healthBarTop = top.y + h - healthBarHeight; int healthBarBottom = top.y + h;
					int red = 255 - (health * 2.55); int green = health * 2.55;

					interfaces::surface->DrawSetColor(red, green, 0, 255);
					if (globals::bHealthBar) {
						interfaces::surface->DrawFilledRect(healthBarLeft, healthBarTop, healthBarRight, healthBarBottom);
					}

					int armorBarWidth = static_cast<int>(w * (armour / 100.0f));

					int armorBarLeft = left;
					int armorBarRight = armorBarLeft + armorBarWidth * 2;
					int armorBarTop = bottom.y + 3;
					int armorBarBottom = armorBarTop + 2;

					interfaces::surface->DrawSetColor(31, 31, 31, 225); 
					if (globals::bArmourBar) {
						interfaces::surface->DrawFilledRect(armorBarLeft, armorBarTop, armorBarLeft + w * 2, armorBarBottom);
					}


					interfaces::surface->DrawSetColor(60, 200, 255, 255);
					if (globals::bArmourBar) {
						interfaces::surface->DrawFilledRect(armorBarLeft, armorBarTop, armorBarRight, armorBarBottom);
					}

				}

				//bones
				CStudioHdr* studio = interfaces::modelInfo->GetStudioModel(entity->GetModel());

				CVector sParent; CVector vParent;
				CVector sChild; CVector vChild;
				if (globals::visualBool[2]) {
					for (int j = 0; j < studio->numBones; j++) {
						mstudiobone_t* pBone = studio->pBone(j);

						if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1)) {
							vChild = entity->GetBonePosition(j);
							vParent = entity->GetBonePosition(pBone->parent);

							interfaces::debugOverlay->ScreenPosition(vParent, sParent);
							interfaces::debugOverlay->ScreenPosition(vChild, sChild);

							int sParentX = static_cast<int>(sParent.x);
							int sParentY = static_cast<int>(sParent.y);
							int sChildX = static_cast<int>(sChild.x);
							int sChildY = static_cast<int>(sChild.y);

							interfaces::surface->DrawSetColor(
								static_cast<int>(globals::boneColor[0] * 255),
								static_cast<int>(globals::boneColor[1] * 255),
								static_cast<int>(globals::boneColor[2] * 255),
								255
							);
							interfaces::surface->DrawLine(sParentX, sParentY, sChildX, sChildY);
						}
					}
				}
			}
		}
	}

	//call original func
	PaintTraverseOriginal(interfaces::panel, vguiPanel, forceRepaint, allowForce);
}