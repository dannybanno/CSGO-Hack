#include "gui.h"

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"

#include <stdexcept>
#include "fonts/font_awesome.h"
#include "../hacks/misc.h"
#include "../core/globals.h"
#include "../core/interfaces.h"
#include <string>
#include <iostream>
#include "../../ext/imgui/imgui_toggle.h"



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
);

// window process
LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	//populate window class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = NULL;

	// register
	if (!RegisterClassEx(&windowClass))
		return false;

	return true;
}

void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		windowClass.lpszClassName,
		windowClass.hInstance
	);
}

bool gui::SetupWindow(const char* windowName) noexcept
{
	//create temp window
	window = CreateWindow(
		windowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		1000,
		1000,
		0,
		0,
		windowClass.hInstance,
		0
	);

	if (!window)
		return false;

	return true;
}

void gui::DestroyWindow() noexcept
{
	if (window)
		DestroyWindow(window);
}

bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return false;

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(
		handle,
		"Direct3DCreate9"
	));

	if (!create)
		return false;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return false;

	D3DPRESENT_PARAMETERS params = { };
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&params,
		&device
	) < 0) return false;

	return true;

}

void gui::DestroyDirectX() noexcept
{
	if (device)
	{
		device->Release();
		device = NULL;
	}

	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}

//setup device
void gui::Setup()
{
	if (!SetupWindowClass("hackClass001"))
		throw std::runtime_error("Failed to create window class");

	if (!SetupWindow("Hack Window"))
		throw std::runtime_error("Failed to create window");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to create device ");

	DestroyWindow();
	DestroyWindowClass();
}

void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	auto params = D3DDEVICE_CREATION_PARAMETERS{ };
	device->GetCreationParameters(&params);

	window = params.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
		);

	ImGui::CreateContext();
	ImGui::StyleColorsLight();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	setup = true;
}

void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//restore wnd proc
	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	DestroyDirectX();
}

ImVec4 RainbowColor(float time)
{
	const float amplitude = 1.5f;
	float r = sinf(gui::frequency * time + 0.0f) * amplitude + 0.5f;
	float g = sinf(gui::frequency * time + 2.0f) * amplitude + 0.5f;
	float b = sinf(gui::frequency * time + 4.0f) * amplitude + 0.5f;
	return ImVec4(r, g, b, 1.0f);
}

float globals::hidden[3] = { 1.f, 0.f, 0.f };
float globals::visible[3] = { 0.f, 0.f, 1.f };
float globals::chamsAlpha = 1.f;

float globals::enemyColour[3] = { 1.f, 0.f, 0.f };
float globals::teamColour[3] = { 0.f, 1.f, 0.f };

float globals::wireColour[3] = { 1.f, 1.f, 1.f };

float globals::aimFOV{ 20.f };

//esp
float globals::espColor[3] = { 1.f, 1.f, 1.f };
float globals::boneColor[3] = { 1.f, 1.f, 1.f };

//visuals
bool globals::visualBool[5] = { false, false, false, false, false };
bool globals::settingsBool[3] = { false, false, false };

bool globals::bHealthBar = false;
bool globals::bArmourBar = false;

//movement
bool globals::movementBool[3] = { false, false, false };

//misc
bool globals::miscBool[1] = { false };

//weapon
bool globals::weaponBool[3] = { false, false, false };

bool globals::bSnaplines = false;
bool globals::bHealthText = false;
bool globals::bFOVCirlce = false;

bool globals::bwireRainbow = false;

static ImVec4 mainColour = ImVec4(255, 0, 255, 0.86);

static bool showMisc = false;
static bool showVisuals = true;
static bool showMovement = false;
static bool showWeapon = false;

//vis
static bool chamsSetting = false;
static bool espSetting = false;
static bool boneESPSetting = false;
static bool wireFrameSetting = false;
static bool glowSetting = false;

const char* globals::drawFromItems[] = { "Bottom", "Center" };
int globals::selectedDrawFrom = 0;

bool globals::boxes = false;
bool globals::corners = false;

const char* globals::diffStyles[] = { "Box Outline", "Corners" };
int globals::selectedStyle = 0;

int globals::cornerSize = 20;

//movement
static bool bhopSettings = false;

//weapon
static bool aimbotSetting = false;

const char* globals::target[] = { "Head", "Neck", "Chest", "Hip", "L|Hand", "R|Hand", "L|Knee", "R|Knee", "L|Foot", "R|Foot"};
int globals::selectedTarget = 0;
int globals::selectedBone = 0;

int globals::targetToBoneIndex[] = { 8, 7, 6, 70, 13, 43, 71, 78, 72, 79  };

float globals::distanceBetweenPlayer = 0;

int globals::playerFOV = 100;

float globals::distanceAim = 500;

float euclidean_distance(float x, float y) {
	return sqrtf((x * x) + (y * y));
}

void gui::Render() noexcept
{
	//screen dimensions
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ::ImGui::GetIO();

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//start
	ImGui::SetNextWindowSize(ImVec2(1100.0f, 700.0f));

	if (open) {

		ImGui::Begin("CSGO Legacy Hack)", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // remove bg

		ImGui::BeginChild("Visuals", ImVec2(220, 0), true, ImGuiWindowFlags_None);

		ImGui::PushFont(io.Fonts->Fonts[1]);

		if (ImGui::Button("Visuals")) {
			showVisuals = true;
			showMovement = false;
			showWeapon = false;
			showMisc = false;
		}
		if (ImGui::Button("Weapon")) {
			showVisuals = false;
			showMovement = false;
			showWeapon = true;
			showMisc = false;
		}
		if (ImGui::Button("Movement")) {
			showMovement = true;
			showVisuals = false;
			showWeapon = false;
			showMisc = false;
		}
		if (ImGui::Button("Misc")) {
			showMovement = false;
			showVisuals = false;
			showWeapon = false;
			showMisc = true;
		}

		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::EndChild();

		ImGui::SetCursorPosX(275);

		if (showVisuals) {

			ImGui::SameLine();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Text("Visuals");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(255, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR", ImVec2(330, 584), true, ImGuiWindowFlags_None);

			ImGui::BeginTable("sections", 2);

			ImGui::TableSetupColumn("title", ImGuiTableColumnFlags_WidthFixed, 190);
			ImGui::TableSetupColumn("setting", ImGuiTableColumnFlags_WidthFixed);

			const char* visualTitles[5] = { "Chams", "ESP", "Bone ESP", "Wire Frame", "Glow"};

			for (int i = 0; i < 5; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
				ImGui::Toggle(visualTitles[i], &globals::visualBool[i], ImGuiToggleFlags_Animated);
				ImGui::PopStyleColor(2);
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
				ImGui::TableSetColumnIndex(1);
				if (i == 0) {
					if (ImGui::Button("Settings")) {
						glowSetting = false;
						espSetting = false;
						boneESPSetting = false;
						bhopSettings = false;
						aimbotSetting = false;
						wireFrameSetting = false;
						chamsSetting = !chamsSetting;
					}
				}
				if (i == 1) {
					if (ImGui::Button("Settings1")) {
						glowSetting = false;
						chamsSetting = false;
						boneESPSetting = false;
						bhopSettings = false;
						aimbotSetting = false;
						wireFrameSetting = false;
						espSetting = !espSetting;
					}
				}
				if (i == 2) {
					if (ImGui::Button("Settings2")) {
						glowSetting = false;
						chamsSetting = false;
						espSetting = false;
						bhopSettings = false;
						aimbotSetting = false;
						wireFrameSetting = false;
						boneESPSetting = !boneESPSetting;
					}
				}
				if (i == 3) {
					if (ImGui::Button("Settings3")) {
						glowSetting = false;
						chamsSetting = false;
						espSetting = false;
						bhopSettings = false;
						aimbotSetting = false;
						boneESPSetting = false;
						wireFrameSetting = !wireFrameSetting;
					}
				}
				if (i == 4) {
					if (ImGui::Button("Settings4")) {
						chamsSetting = false;
						espSetting = false;
						bhopSettings = false;
						aimbotSetting = false;
						boneESPSetting = false;
						wireFrameSetting = false;
						glowSetting = !glowSetting;
					}
				}
			}

			ImGui::EndTable();

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}

		if (showMovement) {

			ImGui::SameLine();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Text("Movement");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(255, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildJ", ImVec2(330, 584), true, ImGuiWindowFlags_None);

			ImGui::BeginTable("sections", 2);

			ImGui::TableSetupColumn("title", ImGuiTableColumnFlags_WidthFixed, 190);
			ImGui::TableSetupColumn("setting", ImGuiTableColumnFlags_WidthFixed);

			const char* movementTitles[3] = { "Bunny Hop" };

			for (int i = 0; i < 1; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
				ImGui::Toggle(movementTitles[i], &globals::movementBool[i], ImGuiToggleFlags_Animated);
				ImGui::PopStyleColor(2);
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
				ImGui::TableSetColumnIndex(1);
				if (i == 0) {
					if (ImGui::Button("Settings")) {
						chamsSetting = false;
						espSetting = false;
						boneESPSetting = false;
						bhopSettings = !bhopSettings;
					}
				}
			}

			ImGui::EndTable();

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}

		if (showWeapon) {

			ImGui::SameLine();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Text("Weapon");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(255, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildQ", ImVec2(330, 584), true, ImGuiWindowFlags_None);

			ImGui::BeginTable("sections", 2);

			ImGui::TableSetupColumn("title", ImGuiTableColumnFlags_WidthFixed, 190);
			ImGui::TableSetupColumn("setting", ImGuiTableColumnFlags_WidthFixed);

			const char* weaponTitles[3] = { "Triggerbot", "Aimbot" };

			for (int i = 0; i < 2; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
				ImGui::Toggle(weaponTitles[i], &globals::weaponBool[i], ImGuiToggleFlags_Animated);
				ImGui::PopStyleColor(2);
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
				ImGui::TableSetColumnIndex(1);
				if (i == 0) {
				}
				if (i == 1) {
					if (ImGui::Button("Settings")) {
						chamsSetting = false;
						espSetting = false;
						boneESPSetting = false;
						bhopSettings = false;
						aimbotSetting = true;
						wireFrameSetting = false;
					}
				}
			}

			ImGui::EndTable();

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		if (showMisc) {

			ImGui::SameLine();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Text("Misc");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(255, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildQ", ImVec2(330, 584), true, ImGuiWindowFlags_None);

			ImGui::BeginTable("sections", 2);

			ImGui::TableSetupColumn("title", ImGuiTableColumnFlags_WidthFixed, 190);
			ImGui::TableSetupColumn("setting", ImGuiTableColumnFlags_WidthFixed);

			const char* miscTitles[1] = { "FOV" };

			ImGui::SliderInt("FOV", &globals::playerFOV, 40, 200);

			for (int i = 0; i < 1; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				if (i = 1) {
					ImGui::SliderInt("FOV", &globals::playerFOV, 40, 200);
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
					ImGui::Toggle(miscTitles[i], &globals::weaponBool[i], ImGuiToggleFlags_Animated);
					ImGui::PopStyleColor(2);
				}
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
				ImGui::TableSetColumnIndex(1);
				if (i == 0) {
				}
			}

			ImGui::EndTable();

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		//chams
		if (chamsSetting && showVisuals && !espSetting && !boneESPSetting && !bhopSettings && !aimbotSetting && !wireFrameSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR4", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			if (ImGui::ColorEdit3("Hidden Color", (float*)&globals::hidden)) {
			}

			if (ImGui::ColorEdit3("Visible Color", (float*)&globals::visible)) {
			}

			ImGui::SliderFloat("Alpha", &globals::chamsAlpha, 0.0f, 1.0f, "%.3f");

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}//chams//chams	//chams
		//glow
		if (glowSetting && showVisuals && !espSetting && !boneESPSetting && !bhopSettings && !aimbotSetting && !wireFrameSetting && !chamsSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR4", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			if (ImGui::ColorEdit3("Team Color", (float*)&globals::teamColour)) {
			}

			if (ImGui::ColorEdit3("Enemy Color", (float*)&globals::enemyColour)) {
			}

			ImGui::SliderFloat("Alpha", &globals::chamsAlpha, 0.0f, 1.0f, "%.3f");

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		//wirefraem
		if (wireFrameSetting && !chamsSetting && showVisuals && !espSetting && !boneESPSetting && !bhopSettings && !aimbotSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR4", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			if (ImGui::ColorEdit3("Wire Colour", (float*)&globals::wireColour)) {
			}

			ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
			ImGui::Toggle("Rainbow", &globals::bwireRainbow, ImGuiToggleFlags_Animated);
			ImGui::PopStyleColor(2);

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		//esp
		if (espSetting && showVisuals && !chamsSetting && !boneESPSetting && !bhopSettings && !aimbotSetting && !wireFrameSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR5", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			if (ImGui::ColorEdit3("Colour", (float*)&globals::espColor)) {

			}

			ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
			ImGui::Toggle("Snaplines", &globals::bSnaplines, ImGuiToggleFlags_Animated);
			ImGui::Toggle("Health Text", &globals::bHealthText, ImGuiToggleFlags_Animated);
			ImGui::PopStyleColor(2);

			if (ImGui::CollapsingHeader("Bars")) {
				ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
				ImGui::Toggle("Health Bar", &globals::bHealthBar, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Armour Bar", &globals::bArmourBar, ImGuiToggleFlags_Animated);
				ImGui::PopStyleColor(2);
			}

			if (ImGui::BeginCombo("Draw From", globals::drawFromItems[globals::selectedDrawFrom])) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = (globals::selectedDrawFrom == i);
					if (ImGui::Selectable(globals::drawFromItems[i], isSelected)) {
						globals::selectedDrawFrom = i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Box Style", globals::diffStyles[globals::selectedStyle])) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = (globals::selectedStyle == i);
					if (ImGui::Selectable(globals::diffStyles[i], isSelected)) {
						globals::selectedStyle = i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::SliderInt("Corner Size", &globals::cornerSize, 1, 40);

			ImGui::EndChild();
			ImGui::PopStyleVar();

		}
		//bones
		if (boneESPSetting && showVisuals && !chamsSetting && !espSetting && !bhopSettings && !aimbotSetting && !wireFrameSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR5", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			if (ImGui::ColorEdit3("Colour", (float*)&globals::boneColor)) {

			}


			ImGui::EndChild();
			ImGui::PopStyleVar();

		}
		//bhop
		if (!boneESPSetting && showVisuals && !chamsSetting && !espSetting && bhopSettings && !aimbotSetting && !wireFrameSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR6", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			ImGui::Text("No Settings!");

			ImGui::EndChild();
			ImGui::PopStyleVar();

		}
		//aimbot
		if (!boneESPSetting && !chamsSetting && !espSetting && !bhopSettings && aimbotSetting && !wireFrameSetting && !glowSetting) {
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::SetCursorPos(ImVec2(600, 55));
			ImGui::Text("Settings");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(600, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildR6", ImVec2(465, 584), true, ImGuiWindowFlags_None);

			ImGui::SliderFloat("FOV", &globals::aimFOV, 5.0f, 255.0f, "%.2f", 0.5f);
			ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
			ImGui::Toggle("FOV Circle", &globals::bFOVCirlce, ImGuiToggleFlags_Animated);
			ImGui::PopStyleColor(2);

			if (ImGui::BeginCombo("Target", globals::target[globals::selectedTarget])) {
				for (int i = 0; i < 10; i++) {
					bool isSelected = (globals::selectedTarget == i);
					if (ImGui::Selectable(globals::target[i], isSelected)) {
						globals::selectedTarget = i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::SliderFloat("Distance", &globals::distanceAim, 1.0f, 750.0f, "%.2f", 1.f);

			globals::selectedBone = globals::targetToBoneIndex[globals::selectedTarget];

			ImGui::EndChild();
			ImGui::PopStyleVar();

		}
	}

	int centerX = screenWidth / 2;
	int centerY = screenHeight / 2;

	// Calculate FOV circle radius
	float fov = globals::aimFOV * (3.14159265359f / 180.0f);
	float radius = tanf(fov / 2) * screenHeight;

	// Draw the FOV circle using ImGui
	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	if (globals::bFOVCirlce) {
		drawList->AddCircle(ImVec2(centerX, centerY), radius, ImColor(255, 0, 0, 255), 64, 2.0f);
	}


	if (globals::localPlayer && interfaces::engine->IsInGame())
	{
		ImFont* font = ImGui::GetFont(); // Get the current font
		float fontSize = 16.0f;

		globals::localPlayer->playerFOV() = globals::playerFOV;
		std::cout << globals::localPlayer->playerFOV();

		for (int i = 1; i < interfaces::globals->maxClients; ++i)
		{
			CEntity* entity = interfaces::entityList->GetEntityFromIndex(i);

			if (!entity)
				continue;

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

			float localPX = globals::localPlayer->GetAbsOrigin().x;
			float localPY = globals::localPlayer->GetAbsOrigin().y;
			float entX = entity->GetAbsOrigin().x;
			float entY = entity->GetAbsOrigin().y;

			int distance = static_cast<int>(euclidean_distance(entX - localPX, entY - localPY) * 0.0254f);

			const auto left = static_cast<int>(top.x - w); const auto right = static_cast<int>(top.x + w);
			int health = 0;
			health = entity->GetHealth();
			if (globals::bHealthText && globals::visualBool[1]) {
				try {
					std::string healthText = std::to_string(health);
					std::string distanceText = std::to_string(distance);

					ImVec2 textPosition(left, top.y - 20);
					ImU32 textColor = IM_COL32_WHITE;

					if (entity->IsDormant() || !entity->IsAlive())
						continue;

					drawList->AddText(font, fontSize, textPosition, textColor, healthText.c_str());
					drawList->AddText(font, fontSize, ImVec2(left + 30, top.y - 20), textColor, distanceText.c_str());
				}
				catch (const std::exception& e) {

				}
			}
		}
	}

	

	ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = mainColour;
	ImGui::GetStyle().Colors[ImGuiCol_Border] = mainColour;

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
)
{
	//toglle menu
	if (GetAsyncKeyState(VK_INSERT) & 1)
		gui::open = !gui::open;

	//pass msg to imgui
	if (ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wideParam,
		longParam
	)) return 1L;

	return CallWindowProc(
		gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}