#pragma once
#include "../../dependencies/common_includes.hpp"

extern IDirect3DStateBlock9 *state_block;

enum esp_info_position {
	RIGHT,
	CENTER_DOWN,
	CENTER_UP
};

struct esp_info_s {
	esp_info_s(const std::string name, const color colors, const int position = RIGHT) {
		f_name = name; f_color = colors; f_position = position;
	}

	std::string f_name;
	int f_position;
	color f_color;
};

class c_menu {
public:
	void run_popup();
	void run();
	void run_visuals_preview();

	void __stdcall create_objects(IDirect3DDevice9* device) {
		if (hooks::window)
			ImGui_ImplDX9_CreateDeviceObjects();
	}

	void __stdcall invalidate_objects() {
		ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void __stdcall setup_resent(IDirect3DDevice9* device) {
		ImGui_ImplDX9_Init(hooks::window, device);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Alpha = 1.0f;
		style.WindowPadding = ImVec2(0, 0);
		style.WindowMinSize = ImVec2(32, 32);
		style.WindowRounding = 5.0f;
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.ChildWindowRounding = 0.0f;
		style.FramePadding = ImVec2(4, 3);
		style.FrameRounding = 4.0f;
		style.ItemSpacing = ImVec2(8, 8);
		style.ItemInnerSpacing = ImVec2(8, 8);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 0.0f;
		style.ScrollbarSize = 6.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 0.0f;
		style.ButtonTextAlign = ImVec2(0.0f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;
		style.AntiAliasedShapes = false;
		style.CurveTessellationTol = 1.f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.11f, 0.28f, 0.92f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.11f, 0.28f, 0.66f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.14f, 0.13f, 0.13f, 0.00f);
		colors[ImGuiCol_ChildWindowBg] = ImVec4(33 / 255.f, 35 / 255.f, 47 / 255.f, 1.0f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.15f, 0.31f, 0.83f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.14f, 0.39f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.17f, 0.14f, 0.39f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.70f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.02f, 0.88f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.12f, 0.64f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.19f, 0.11f, 0.54f, 0.53f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.07f, 0.24f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.13f, 0.66f, 0.76f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.19f, 0.09f, 0.82f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.14f, 0.39f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.14f, 0.39f, 1.00f); //
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.16f, 0.78f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.34f, 0.81f, 0.19f, 0.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.71f, 0.05f, 0.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.65f, 0.16f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.31f, 0.77f, 0.07f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.37f, 0.71f, 0.06f, 0.00f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.16f, 0.78f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.34f, 0.81f, 0.19f, 0.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.71f, 0.05f, 0.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.65f, 0.16f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.31f, 0.77f, 0.07f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.37f, 0.71f, 0.06f, 0.00f);
		colors[ImGuiCol_CloseButton] = ImVec4(0, 0, 0, 0);
		colors[ImGuiCol_CloseButtonHovered] = ImVec4(0, 0, 0, 0);
		colors[ImGuiCol_HotkeyOutline] = ImVec4(0, 0, 0, 0);

		create_objects(device);
	}

	void apply_fonts() {
		ImGui::CreateContext();

		font_main = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 18);
		font_menu = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 12);
	}

	void __stdcall end_present(IDirect3DDevice9* device) {
		if (opened) {
			if (ImGui::GetStyle().Alpha > 1.f)
				ImGui::GetStyle().Alpha = 1.f;
			else if (ImGui::GetStyle().Alpha != 1.f)
				ImGui::GetStyle().Alpha += 0.05f;
		}

		ImGui::Render();

		state_block->Apply();
		state_block->Release();
	}

	void __stdcall pre_render(IDirect3DDevice9* device) {
		D3DVIEWPORT9 d3d_viewport;
		device->GetViewport(&d3d_viewport);

		device->CreateStateBlock(D3DSBT_ALL, &state_block);
		state_block->Capture();

		device->SetVertexShader(nullptr);
		device->SetPixelShader(nullptr);
		device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_STENCILENABLE, FALSE);

		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

		device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
	}

	void __stdcall post_render() {
		ImGui_ImplDX9_NewFrame();
	}
	ImFont* font_main;
	ImFont* font_menu;
	ImFont* font_main_caps;
	bool opened = false;
private:
	ImDrawData _drawData;
	DWORD dwOld_D3DRS_COLORWRITEENABLE;
};

extern c_menu menu;