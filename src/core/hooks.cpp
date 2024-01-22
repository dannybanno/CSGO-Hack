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
#include "../../ext/imgui/imgui_notify.h"

#define _CRT_SECURE_NO_WARNINGS

#define MAXSTUDIOBONES 128
#define BONE_USED_BY_HITBOX 0x100

int screenHeight = GetSystemMetrics(SM_CYSCREEN); int screenWidth = GetSystemMetrics(SM_CXSCREEN);

HFont mainFont;
HFont weaponIcon;


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

	//MH_CreateHook(
	//	memory::Get(interfaces::clientMode, 18),
	//	&OverrideView,
	//	reinterpret_cast<void**>(&OverrideViewOriginal)
	//);

	MH_CreateHook(
		memory::Get(interfaces::client, 36),
		&FrameStageNotify,
		reinterpret_cast<void**>(&FrameStageNotifyOriginal)
	);

	MH_EnableHook(MH_ALL_HOOKS);

	mainFont = interfaces::surface->CreateFont1();
	interfaces::surface->SetFontGlyphSet(mainFont, "Tahoma", 17, 800, 0, 0, FONTFLAG_OUTLINE);

	weaponIcon = interfaces::surface->CreateFont1();
	interfaces::surface->SetFontGlyphSet(weaponIcon, "csgo_icons", 17, 800, 0, 0, NULL);

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

	ImGuiToast toast(ImGuiToastType_Success, 9000);
	toast.set_title("Success");
	toast.set_content("Welcome to fart.club, dannybanno.github.io");
	ImGui::InsertNotification(toast);

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
	//static uintptr_t sequence = (uintptr_t)memory::PatternScan("client.dll", "FF 23");
	const auto result = x86RetSpoof::invokeStdcall<bool>((uintptr_t)hooks::CreateMoveOriginal, sequence, frameTime, cmd);

	// make sure this function is being called from CInput::CreateMove
	if (!cmd || !cmd->commandNumber)
		return result;

	// this would be done anyway by returning true
	if (CreateMoveOriginal(interfaces::clientMode, frameTime, cmd))
		interfaces::engine->SetViewAngles(cmd->viewAngles);

	// get our local player here
	globals::UpdateLocalPlayer();
		
	if (globals::localPlayer && globals::localPlayer->IsAlive() && interfaces::engine->IsInGame())
	{
		if (globals::localPlayer && globals::localPlayer->IsAlive()) {
			if (globals::movementBool[0]) {
				hacks::RunBunnyHop(cmd);
			}
			if (globals::movementBool[1]) {
				hacks::RunSpinbot(cmd);
			}
			if (globals::weaponBool[0]) {
				hacks::TriggerBot(cmd);
			}
			if (globals::weaponBool[1]) {
				hacks::RunAimbot(cmd);
			}
		}

	}


	return false;
}



void __stdcall hooks::FrameStageNotify(ClientFrameStage_t stage)
{
	if (stage == FRAME_RENDER_START) {

		
	}

	

	FrameStageNotifyOriginal(interfaces::client, stage);
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
	if (globals::localPlayer && info.renderable && interfaces::engine->IsInGame()) {
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
			//static IMaterial* wireMaterial = interfaces::materialSystem->FindMaterial("models/player/ct_fbi/ct_fbi_glass");
			wireMaterial->SetMaterialVarFlag(IMaterial::MATERIAL_VAR_WIREFRAME, true);
			interfaces::studioRender->ForcedMaterialOverride(wireMaterial);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			return interfaces::studioRender->ForcedMaterialOverride(nullptr);
		}

		if (entity && entity->IsPlayer() && entity->GetTeam() != globals::localPlayer->GetTeam() && globals::visualBool[0]) {
			static IMaterial* material = interfaces::materialSystem->FindMaterial("debug/debugambientcube");

			interfaces::studioRender->SetAlphaModulation(globals::chamsAlpha);

			//thru wall
			material->SetMaterialVarFlag(IMaterial::MATERIAL_VAR_IGNOREZ, true);
			material->SetMaterialVarFlag(IMaterial::MATERIAL_VAR_WIREFRAME, false);
			interfaces::studioRender->SetColorModulation(globals::hidden);
			interfaces::studioRender->ForcedMaterialOverride(material);
			DrawModelOriginal(interfaces::studioRender, results, info, bones, flexWeights, flexDelayedWeights, modelOrigin, flags);

			//no wall
			material->SetMaterialVarFlag(IMaterial::MATERIAL_VAR_IGNOREZ, false);
			material->SetMaterialVarFlag(IMaterial::MATERIAL_VAR_WIREFRAME, false);
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

//void __stdcall hooks::OverrideView(ViewSetup* setup) noexcept
//{
//	if (globals::localPlayer && interfaces::engine->IsInGame())
//	{
//		setup->viewModelFOV = 150;
//		setup->fov = 130;
//
//		if (globals::visualBool[6]) {
//			globals::localPlayer->flashDuration() = 0;
//		}
//	}
//
//	OverrideViewOriginal(interfaces::clientMode, setup);
//}


float euclidean_distance(float x, float y) {
	return sqrtf((x * x) + (y * y));
}

void Text22(int x, int y, Color clr, std::string text, bool center = false)
{
	std::wstring str(text.begin(), text.end());

	if (center) {
		int textWidth, textHeight;
		interfaces::surface->GetTextSize(mainFont, str.c_str(), textWidth, textHeight);
		x -= textWidth / 2;
		y -= textHeight / 2;
	}

	interfaces::surface->DrawSetTextPos(x, y);
	interfaces::surface->DrawSetTextColor(clr);
	interfaces::surface->DrawSetTextFont(mainFont);
	interfaces::surface->DrawPrintText(str.c_str(), str.length());
}

CVector get_text_size22(unsigned long font, std::string text) {
	std::wstring a(text.begin(), text.end());
	const wchar_t* wstr = a.c_str();
	static int width, height;

	interfaces::surface->get_text_size(font, wstr, width, height);
	return { static_cast<float>(width), static_cast<float>(height) };
}


void __stdcall hooks::PaintTraverse(std::uintptr_t vguiPanel, bool forceRepaint, bool allowForce) noexcept
{

	if (vguiPanel == interfaces::engineVGui->GetPanel(PANEL_TOOLS))
	{
		globals::UpdateLocalPlayer();

		if (globals::localPlayer && interfaces::engine->IsInGame() && globals::localPlayer->IsAlive())
		{
			if (!interfaces::engine->IsInGame() || !globals::localPlayer)
				return;

			std::string spectators;

			for (int i = 1; i <= interfaces::entityList->GetHighestEntity(); ++i)
			{

				CEntity* entity = interfaces::entityList->GetEntityFromIndex(i);

				if (!entity)
					continue;

				std::string wpnName = "";

				switch (entity->GetClientClass()->classID) {
				case CClientClass::CAK47:
					wpnName = std::string("AK-47");
					break;
				case CClientClass::CWeaponAWP:
					wpnName = std::string("AWP");
					break;
				case CClientClass::CDEagle:
					wpnName = std::string("Desert Eagle");
					break;
				case CClientClass::CWeaponElite:
					wpnName = std::string("Dual Berettas");
					break;
				case CClientClass::CWeaponFiveSeven:
					wpnName = std::string("Five-SeveN");
					break;
				case CClientClass::CWeaponGlock:
					wpnName = std::string("Glock-18");
					break;
				case CClientClass::CWeaponAug:
					wpnName = std::string("AUG");
					break;
				case CClientClass::CDecoyProjectile:
					wpnName = std::string("Decoy Projectile");
					break;
				case CClientClass::CFlashbang:
					wpnName = std::string("Flashbang");
					break;
				case CClientClass::CHEGrenade:
					wpnName = std::string("High-Explosive Grenade");
					break;
				case CClientClass::CInferno:
					wpnName = std::string("Inferno");
					break;
				case CClientClass::CIncendiaryGrenade:
					wpnName = std::string("Incendiary Grenade");
					break;
				case CClientClass::CMolotovProjectile:
					wpnName = std::string("Molotov Projectile");
					break;
				case CClientClass::CSmokeGrenade:
					wpnName = std::string("Smoke Grenade");
					break;
				case CClientClass::CSnowballProjectile:
					wpnName = std::string("Snowball Projectile");
					break;
				case CClientClass::CWeaponNOVA:
					wpnName = std::string("Nova");
					break;
				case CClientClass::CWeaponP250:
					wpnName = std::string("P250");
					break;
				case CClientClass::CWeaponSCAR20:
					wpnName = std::string("SCAR-20");
					break;
				case CClientClass::CWeaponSG556:
					wpnName = std::string("SG 553");
					break;
				case CClientClass::CWeaponSSG08:
					wpnName = std::string("SSG 08");
					break;
				case CClientClass::CC4:
					wpnName = std::string("C4 Explosive");
					break;
				case CClientClass::CWeaponM4A1:
					wpnName = std::string("M4A1");
					break;
				case CClientClass::CWeaponUMP45:
					wpnName = std::string("UMP-45");
					break;
				case CClientClass::CWeaponXM1014:
					wpnName = std::string("XM1014");
					break;
				case CClientClass::CWeaponBizon:
					wpnName = std::string("PP-Bizon");
					break;
				case CClientClass::CWeaponMag7:
					wpnName = std::string("MAG-7");
					break;
				case CClientClass::CWeaponNegev:
					wpnName = std::string("Negev");
					break;
				case CClientClass::CWeaponSawedoff:
					wpnName = std::string("Sawed-Off");
					break;
				case CClientClass::CWeaponTec9:
					wpnName = std::string("Tec-9");
					break;
				case CClientClass::CWeaponTaser:
					wpnName = std::string("Taser");
					break;
				case CClientClass::CWeaponHKP2000:
					wpnName = std::string("P2000");
					break;
				case CClientClass::CWeaponMP7:
					wpnName = std::string("MP7");
					break;
				case CClientClass::CWeaponMP9:
					wpnName = std::string("MP9");
					break;
				case CClientClass::CWeaponUSP:
					wpnName = std::string("USP-S");
					break;
				case CClientClass::CWeaponFamas:
					wpnName = std::string("Famas");
					break;
				case CClientClass::CWeaponGalil:
					wpnName = std::string("Galil");
					break;
				case CClientClass::CWeaponGalilAR:
					wpnName = std::string("Galil");
					break;
				case CClientClass::CWeaponG3SG1:
					wpnName = std::string("G3SG1");
					break;
				case CClientClass::CWeaponP90:
					wpnName = std::string("P90");
					break;
				case CClientClass::CPlantedC4:
					wpnName = std::string("C4 Planted");
					break;
				default:
					wpnName = std::string("");
					break;
				}

				int owner = entity->GetOwner();

				if (wpnName != "" && owner == -1) {
					CVector weaponPos;
					interfaces::debugOverlay->ScreenPosition(entity->GetAbsOrigin(), weaponPos);
					Text22(weaponPos.x, weaponPos.y, Color(255, 255, 255, 255), wpnName);
				}

				if (wpnName == "Flashbang" || wpnName == "Decoy Projectile" || wpnName == "High-Explosive Grenade"
					|| wpnName == "Inferno" || wpnName == "Incendiary Grenade" || wpnName == "Molotov Projectile"
					|| wpnName == "Smoke Grenade" || wpnName == "Snowball Projectile") {
					CVector weaponPos;
					interfaces::debugOverlay->ScreenPosition(entity->GetAbsOrigin(), weaponPos);
					Text22(weaponPos.x, weaponPos.y, Color(255, 255, 0, 255), wpnName);
				}

				if (entity->IsAlive())
					continue;

				if (entity->IsDormant())
					continue;

				if (!entity->m_hObserverTarget())
					continue;


				//CEntity* target = entity->m_hObserverTarget();
				//
				//if (!target->IsPlayer())
				//	continue;
				//
				//PlayerInfo entityinfo ;
				//PlayerInfo targetinfo ;
				//
				//interfaces::engine->GetPlayerInfo(i, &entityinfo);
				//interfaces::engine->GetPlayerInfo(i, &targetinfo);
				//
				//spectators += std::string(entityinfo.name) + " > " + targetinfo.name + "\n";
				//
				//Text22(10, 200, Color(255, 255, 255, 255), spectators.c_str());

			}

			for (int i = 1; i <= interfaces::globals->maxClients; ++i)
			{
				PlayerInfo info;

				CEntity* entity = interfaces::entityList->GetEntityFromIndex(i);

				if (!entity)
					continue; 

				if(!globals::localPlayer)
					continue;

				if (entity->IsDormant() || !entity->IsAlive())
					continue;

				if (entity->GetTeam() == globals::localPlayer->GetTeam())
					continue;

				if (!globals::localPlayer->IsAlive()) {

					const auto target = globals::localPlayer->GetObserverTarget();

					if (target == entity)
						continue;
				}

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

				CVector localEyePos;

				if (globals::visualBool[1] && entity->IsAlive()) {

					globals::localPlayer->GetEyePosition(localEyePos);

					CTrace trace;
					interfaces::engineTrace->TraceRay(
						CRay{ localEyePos, bones[7].Origin() },
						MASK_SHOT,
						{ globals::localPlayer },
						trace
					);

					if (!trace.entity || trace.fraction < 0.97f) {
						interfaces::surface->DrawSetColor(255, 0, 0, 255);
					}
					else {
						interfaces::surface->DrawSetColor(0, 255, 0, 255);
					}

					//interfaces::surface->DrawSetColor(
					//	static_cast<int>(globals::espColor[0] * 255),
					//	static_cast<int>(globals::espColor[1] * 255),
					//	static_cast<int>(globals::espColor[2] * 255),
					//	255
					//);
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

				}

				if (globals::bHealthText && entity->IsAlive()) {

					float localPX = globals::localPlayer->GetAbsOrigin().x;
					float localPY = globals::localPlayer->GetAbsOrigin().y;
					float entX = entity->GetAbsOrigin().x;
					float entY = entity->GetAbsOrigin().y;

					int distance = static_cast<int>(euclidean_distance(entX - localPX, entY - localPY) * 0.0254f);

					std::string healthText = std::to_string(health);
					std::string distanceText = std::to_string(distance) + "M";
					std::string moneyText = "$" + std::to_string(entity->GetDosh());


					interfaces::engine->GetPlayerInfo(i, &info);

					std::string playerName = info.name;

					
					float baseFontSize = 17.0f;  

					float scaleFactor = 0.01f;  

					float scaledFontSize = baseFontSize + distance * scaleFactor;


					interfaces::surface->SetFontGlyphSet(mainFont, "Tahoma", scaledFontSize, 800, 0, 0, FONTFLAG_OUTLINE);

					//wchar_t name[128];
					//
					//if (MultiByteToWideChar(CP_UTF8, 0, info.name, -1, name, 128) < 0)
					//	continue;
					//
					//int wei = 0, hei = 0;
					//interfaces::surface->GetTextSize(mainFont, name, wei, hei);
					//static POINT txtSize = GetTextSize1(mainFont, playerName.c_str());
					//std::cout << txtSize.x << " ::: " << txtSize.y << std::endl;
					CVector nameSize = get_text_size22(mainFont, playerName.c_str());

					float textCenterX = top.x -= (nameSize.x * 0.5f);

					Text22(textCenterX, top.y - 20, Color(255, 255, 255, 255), playerName.c_str());

					Text22(right + 5, top.y, Color(0, 185, 0, 255), moneyText.c_str());
					Text22(right + 5, top.y + 15, Color(255, 255, 255, 255), healthText.c_str());
					Text22(right + 5, top.y + 30, Color(255, 255, 0, 255), distanceText.c_str());
					if (entity->IsScoped()) { Text22(right + 5, top.y + 45, Color(255, 255, 255, 255), "Scoped"); }

					if (entity->HasC4() == entity->GetIndex()) { Text22(right + 5, top.y + 60, Color(255, 255, 255, 255), "C4"); }

					if (entity->IsDefusing()) { Text22(right + 5, top.y + 75, Color(255, 255, 255, 255), "Defusing"); }

					CVector defuseSize = get_text_size22(mainFont, "Defusing");
					float warnCenterX = screenCenterX - (defuseSize.x * 0.5f);

					if (entity->IsDefusing()) { Text22(warnCenterX, screenCenterY + 30, Color(255, 255, 255, 255), "Defusing"); }

					CEntity* activeWeapon = entity->GetActiveWeapon();

					bool noWpn = true;

					if (!activeWeapon)
						return;

					const int weaponType = activeWeapon->GetWeaponType(); 

					std::string clip1 = std::to_string(activeWeapon->GetClip());
					std::string ammoReserve = std::to_string(activeWeapon->m_iPrimaryReserveAmmoCount());

					std::string wpnText = activeWeapon->GetWeaponName() + " [" + clip1 + "/" + ammoReserve + "]";

					CVector wpntxtSize = get_text_size22(mainFont, wpnText.c_str());

					float wpntextCenterX = rectCenterX -= (wpntxtSize.x * 0.5f );

					Text22(wpntextCenterX, bottom.y + 5, Color(255, 255, 0, 255), wpnText.c_str());

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

				//bones
				CStudioHdr* studio = interfaces::modelInfo->GetStudioModel(entity->GetModel());

				CVector sParent; CVector vParent;
				CVector sChild; CVector vChild;
				if (globals::visualBool[2] && entity->IsAlive()) {
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