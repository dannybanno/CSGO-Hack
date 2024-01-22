#include "gui.h"

#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx9.h"
#include "../../ext/imgui/imgui_toggle.h"

#include "../../ext/imgui/imgui_notify.h"
#include "../../ext/imgui/tahoma.h"

#include <stdexcept>
#include "fonts/font_awesome.h"
#include "../hacks/misc.h"
#include "../core/globals.h"
#include "../core/interfaces.h"
#include <string>
#include <iostream>
#include <algorithm>

#include <vector>
#include <fstream>
#include "../menu/config/nlohmann/json.hpp"


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
bool globals::visualBool[6] = { false, false, false, false, false, false };
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
bool globals::bSilentAim = true;
bool globals::bTargetThreat = false;

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

const char* globals::target[] = { "Head", "Neck", "Chest", "Hip", "L|Hand", "R|Hand", "L|Knee", "R|Knee", "L|Foot", "R|Foot" };
int globals::selectedTarget = 0;
int globals::selectedBone = 0;

int globals::targetToBoneIndex[] = { 8, 7, 6, 70, 13, 43, 71, 78, 72, 79 };

float globals::distanceBetweenPlayer = 0;

int globals::playerFOV = 90;

float globals::distanceAim = 500;

const char* visualTitles[6] = { "Chams", "ESP", "Bone ESP", "Wire Frame", "Glow", "No Flash" };
const char* movementTitles[3] = { "Bunny Hop", "Spinbot" };
const char* weaponTitles[3] = { "Triggerbot", "Aimbot" };
const char* miscTitles[1] = { "FOV" };

bool bShowLogs = true;

#define IM_PRINTFARGS(fmt_index) __attribute__((format(printf, fmt_index, fmt_index+1)))

struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        
    bool                ScrollToBottom;

    void    Clear()     { Buf.clear(); LineOffsets.clear(); }

	void AddLog(const char* prefix, const char* fmt, ...)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);

		struct tm timeinfo;
		localtime_s(&timeinfo, &now_c);
		char timeStr[64];
		strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

		std::ostringstream logStream;
		logStream << prefix << " | " << timeStr << " | ";

		int old_size = Buf.size();

		va_list args;
		va_start(args, fmt);
		logStream << fmt;

		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), logStream.str().c_str(), args);

		va_end(args);

		Buf.append(buffer);

		int new_size = Buf.size();
		if (old_size > 0 && Buf[old_size - 1] != '\n') {
			LineOffsets.push_back(old_size);
		}

		// Save the offset of the start of the next line
		if (new_size > old_size && Buf[new_size - 1] == '\n') {
			LineOffsets.push_back(new_size);
		}

		ScrollToBottom = true;
	}

	void SaveLogsToFile(const char* filename)
	{
		std::ofstream file(filename);
		if (!file.is_open())
		{
			return;
		}

		const char* buf_begin = Buf.begin();
		const char* line = buf_begin;
		for (int line_no = 0; line != NULL; line_no++)
		{
			const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
			if (line_end)
				file.write(line, line_end - line + 1); // Include the newline character
			line = line_end && line_end[1] ? line_end + 1 : NULL;
		}

		file.close();
	}

	void    Draw(const char* title, bool* p_opened = NULL)
	{
		ImGui::SetNextWindowSize(ImVec2(450, 300));
		ImGui::Begin(title, p_opened);
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::NewLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling");
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

		if (copy) ImGui::LogToClipboard();

		if (Filter.IsActive())
		{
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != NULL; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
				if (Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : NULL;
			}
		}
		else
		{
			ImGui::TextUnformatted(Buf.begin());
		}

		if (ScrollToBottom)
			ImGui::SetScrollY(1.0f);
		ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}
};

namespace config {
	std::string path = "C:\\ProgramData\\fartClub_cfg";

	//configs stored in list
	std::vector<std::string> list;

	int selectedConfig = 0;

	template<typename Type>
	void try_val(nlohmann::ordered_json& j, std::string name, Type& _value) {

		if (name.empty())
			return;

		if (j.dump().find(name) == std::string::npos)
			return;

		j.at(name).get_to(_value);
	}

	bool refresh() {
		if (!std::filesystem::exists(path))
			std::filesystem::create_directory(path);

		list.clear();
		selectedConfig = 0;

		for (auto file : std::filesystem::directory_iterator(path)) {

			//narrow down to "name.json"
			//narrow down to just "name"
			std::string file_name = file.path().filename().string();

			if (size_t location = file_name.find(".json"); location != std::string::npos)
				file_name.replace(file_name.begin() + location, file_name.end(), "");

			if (!file_name.empty())
				list.push_back(file_name);

			printf("%s\n", file_name.c_str());

		}

		return true;
	}

	bool load(std::string name) {
		if (name.empty())
			return false;

		auto _path = std::string(path) + "\\" + name + ".json";

		if (!std::filesystem::exists(_path))
			return false;

		std::ifstream read_stream(_path);
		nlohmann::ordered_json j = nlohmann::ordered_json::parse(read_stream, 0, false);

		try_val(j, "chams", globals::visualBool[0]);
		try_val(j, "chams_vis", globals::visible);
		try_val(j, "chams_hid", globals::hidden);
		try_val(j, "chams_alph", globals::chamsAlpha);

		try_val(j, "esp", globals::visualBool[1]);
		try_val(j, "esp_clr", globals::espColor);
		try_val(j, "esp_hp_bar", globals::bHealthBar);
		try_val(j, "esp_arm_bar", globals::bArmourBar);
		try_val(j, "esp_text", globals::bHealthText);
		try_val(j, "esp_draw_from", globals::selectedDrawFrom);
		try_val(j, "esp_box", globals::selectedStyle);
		try_val(j, "esp_corner", globals::cornerSize);
		try_val(j, "esp_snaplines", globals::bSnaplines);

		try_val(j, "bones", globals::visualBool[2]);
		try_val(j, "bones_clr", globals::boneColor);

		try_val(j, "wireframe", globals::visualBool[3]);
		try_val(j, "wireframe_rbow", globals::bwireRainbow);
		try_val(j, "wireframe_clr", globals::wireColour);

		try_val(j, "glow", globals::visualBool[4]);
		try_val(j, "glow_en_clr", globals::enemyColour);
		try_val(j, "glow_tm_clr", globals::teamColour);

		try_val(j, "flash", globals::visualBool[5]);

		try_val(j, "bunny", globals::movementBool[0]);
		try_val(j, "spin", globals::movementBool[1]);

		try_val(j, "triggerbot", globals::weaponBool[0]);

		try_val(j, "aimbot", globals::weaponBool[1]);
		try_val(j, "aimbot_fov", globals::aimFOV);
		try_val(j, "aimbot_silent", globals::bSilentAim);
		try_val(j, "aimbot_distance", globals::distanceAim);
		try_val(j, "aimbot_target", globals::selectedTarget);
		try_val(j, "aimbot_threat", globals::bTargetThreat);
		try_val(j, "aimbot_fov_circ", globals::bFOVCirlce);

		try_val(j, "player_fov", globals::playerFOV);

		read_stream.close();

		config::refresh();

		return true;
	}

	bool save(std::string name) {
		if (name.empty())
			return false;

		auto _path = std::string(path) + "\\" + name + ".json";
		nlohmann::ordered_json j = nlohmann::ordered_json{
			{ "chams", globals::visualBool[0] },	
			{ "chams_vis", globals::visible },
			{ "chams_hid", globals::hidden },
			{ "chams_alph", globals::chamsAlpha },

			{ "esp", globals::visualBool[1] },			 
			{ "esp_clr", globals::espColor },
			{ "esp_hp_bar", globals::bHealthBar },
			{ "esp_arm_bar", globals::bArmourBar },
			{ "esp_text", globals::bHealthText },
			{ "esp_draw_from", globals::selectedDrawFrom },
			{ "esp_box", globals::selectedStyle },
			{ "esp_corner", globals::cornerSize },
			{ "esp_snaplines", globals::bSnaplines },

			{ "bones", globals::visualBool[2] },		 
			{ "bones_clr", globals::boneColor },	

			{ "wireframe", globals::visualBool[3] },	
			{ "wireframe_rbow", globals::bwireRainbow },	
			{ "wireframe_clr", globals::wireColour },	

			{ "glow", globals::visualBool[4] },			 
			{ "glow_en_clr", globals::enemyColour },	
			{ "glow_tm_clr", globals::teamColour },	

			{ "flash", globals::visualBool[5] },		 

			{ "bunny", globals::movementBool[0] },	
			{ "spin", globals::movementBool[1] },	

			{ "triggerbot", globals::weaponBool[0] },	

			{ "aimbot", globals::weaponBool[1] },	
			{ "aimbot_fov", globals::aimFOV },
			{ "aimbot_silent", globals::bSilentAim },
			{ "aimbot_distance", globals::distanceAim },
			{ "aimbot_target", globals::selectedTarget },
			{ "aimbot_threat", globals::bTargetThreat },
			{ "aimbot_fov_circ", globals::bFOVCirlce },

			{ "player_fov", globals::playerFOV }

		};

		std::ofstream output(_path);
		output << j.dump(4);
		output.close();

		if (!std::filesystem::exists(_path))
			return false;

		config::refresh();

		return true;
	}

	bool remove(std::string name) {
		if (name.empty())
			return false;

		auto _path = std::string(path) + "\\" + name + ".json";
		if (!std::filesystem::exists(_path))
			return false;

		std::filesystem::remove(_path);
		config::refresh();

		return true;
	}

}



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

	config::refresh();

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

//float euclidean_distance(float x, float y) {
//	return sqrtf((x * x) + (y * y));
//}

int index = 0;

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

		ImGui::Begin("fart.club", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // remove bg

		ImGui::BeginChild("Visuals", ImVec2(220, 0), true, ImGuiWindowFlags_None);

		ImGui::PushFont(io.Fonts->Fonts[1]);

		if (ImGui::Button("Visuals")) {
			index = 0;
		}
		if (ImGui::Button("Weapon")) {
			index = 1;
		}
		if (ImGui::Button("Movement")) {
			index = 2;
		}
		if (ImGui::Button("Misc")) {
			index = 3;
		}
		if (ImGui::Button("Config")) {
			index = 4;
		}

		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::SetCursorPosX(275);

		if (index == 0) {	//Visuals

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

			for (int i = 0; i < 6; i++) {
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
			ImGui::PopStyleVar(2);
		}

		if (index == 2) {

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

			for (int i = 0; i < 2; i++) {
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
			ImGui::PopStyleVar(2);
		}

		if (index == 1) {

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
			ImGui::PopStyleVar(2);
		}
		if (index == 3) {

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

			ImGui::SliderInt("FOV", &globals::playerFOV, 40, 200);

			for (int i = 0; i < 1; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				if (i == 0) {
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
			ImGui::PopStyleVar(2);
		}
		if (index == 4) {

			ImGui::SameLine();
			ImGui::PushFont(io.Fonts->Fonts[1]);
			ImGui::Text("Config");
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::SetCursorPos(ImVec2(255, 95));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
			ImGui::BeginChild("ChildU", ImVec2(330, 584), true, ImGuiWindowFlags_None);

			auto size = config::list.size();
			for (int i = 0; i < size; i++) {
				auto cfg_name = config::list[i];
				bool selected = (i == config::selectedConfig);

				if (ImGui::Selectable(cfg_name.c_str(), &selected))
					config::selectedConfig = i;

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			static char buffer[64]{};

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextWithHint("##input_text", "Config name:", buffer, sizeof(buffer));
			ImGui::PopItemWidth();

			if (ImGui::Button("Load", ImVec2(ImGui::GetContentRegionAvail().x / 2, 35.0f))) {
				if (strlen(buffer) > 1)
					config::load(buffer);
				else
					config::load(config::list[config::selectedConfig]);
			}

			ImGui::SameLine(0, 3);

			if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x , 35.0f))) {
				if (strlen(buffer) > 1)
					config::save(buffer);
				else
					config::save(config::list[config::selectedConfig]);
			}

			if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x / 2, 35.0f))) {
				if (strlen(buffer) > 3)
					config::remove(buffer);
				else
					config::remove(config::list[config::selectedConfig]);
			}
			ImGui::SameLine(0, 3);
			if (ImGui::Button("Refresh", ImVec2(ImGui::GetContentRegionAvail().x, 35.0f))) {
				config::refresh();
			}

			if (ImGui::Button("Open Folder", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				system("explorer C:\\ProgramData\\fartClub_cfg");
			}

			if (ImGui::Button("Open / Close Logs", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				bShowLogs = !bShowLogs;
			}

			ImGui::EndChild();
			ImGui::PopStyleVar(2);
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
			ImGui::PopStyleVar(2);
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
			ImGui::PopStyleVar(2);
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
			ImGui::PopStyleVar(2);
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
			ImGui::PopStyleVar(2);

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
			ImGui::PopStyleVar(2);

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
			ImGui::PopStyleVar(2);

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

			ImGui::PushStyleColor(ImGuiCol_Button, mainColour);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mainColour);
			ImGui::Toggle("Silent", &globals::bSilentAim, ImGuiToggleFlags_Animated);
			ImGui::Toggle("Target Threat", &globals::bTargetThreat, ImGuiToggleFlags_Animated);
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(2);

			ImGui::EndChild();

		}
	}

	static ExampleAppLog my_log;

	my_log.AddLog("[INFO]", "Test 123 \n");

	if (bShowLogs) {
		my_log.Draw("Logs");
	}

	int centerX = screenWidth / 2;
	int centerY = screenHeight / 2;

	float fov = globals::aimFOV * (3.14159265359f / 180.0f);
	float radius = tanf(fov / 2) * screenHeight;

	
	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	if (globals::bFOVCirlce) {
		drawList->AddCircle(ImVec2(centerX, centerY), radius, ImColor(255, 0, 0, 255), 64, 2.0f);
	}

	ImFont* font = ImGui::GetFont(); 
	float fontSize = 16.0f;

	ImU32 textColor = IM_COL32_WHITE; 

	const char* allTitles[] = {
		visualTitles[0], visualTitles[1], visualTitles[2], visualTitles[3], visualTitles[4], visualTitles[5],
		movementTitles[0], movementTitles[1],
		weaponTitles[0], weaponTitles[1]
	};

	int totalTitles = sizeof(allTitles) / sizeof(allTitles[0]);

	struct TitleInfo {
		const char* title;
		int index;
	};

	TitleInfo titleInfos[10];

	for (int i = 0; i < totalTitles; ++i) {
		titleInfos[i].title = allTitles[i];
		titleInfos[i].index = i;
	}

	std::sort(titleInfos, titleInfos + totalTitles, [](const TitleInfo& a, const TitleInfo& b) {
		return std::strlen(b.title) < std::strlen(a.title);
	});

	float enabledFontSz = 30.0f;

	int count = 0;
	for (int i = 0; i < totalTitles; ++i) {
		int originalIndex = titleInfos[i].index;

		if ((originalIndex < 6 && globals::visualBool[originalIndex]) ||
			(originalIndex >= 6 && originalIndex < 8 && globals::movementBool[originalIndex - 6]) ||
			(originalIndex >= 8 && originalIndex < 10 && globals::weaponBool[originalIndex - 8])) {

			const char* text = allTitles[originalIndex];
			ImVec2 textSize = ImGui::CalcTextSize(text);

			ImVec2 textPosition = ImVec2(100, 100 + count * enabledFontSz);

			drawList->AddText(font, enabledFontSz, textPosition, textColor, text);

			count++;
		}
	}

	//Radar();

	ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = mainColour;
	ImGui::GetStyle().Colors[ImGuiCol_Border] = mainColour;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.f); // Round borders
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(10.f / 255.f, 10.f / 255.f, 10.f / 255.f, 100.f / 255.f)); // border color
	ImGui::RenderNotifications(); // <-- Here we render all notifications
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(2);

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

