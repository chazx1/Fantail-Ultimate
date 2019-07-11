#include "menu.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "../features/misc/logs.hpp"
#include "../features/skinchanger/parser.hpp"

c_menu menu;

#define UNLEN 256
IDirect3DStateBlock9 *state_block;
bool reverse = false;
int offset = 0;
bool show_popup = false;
bool save_config = false;
bool load_config = false;
namespace ImGui {
	long get_mils() {
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	}

	void begin_popup(const char* text, int on_screen_mils, bool* done) {
		if (!done)
			show_popup = true;

		ImGuiIO &io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		int width = io.DisplaySize.x;
		static long old_time = -1;
		ImGui::SetNextWindowPos(ImVec2(width - offset, 100));
		style.WindowMinSize = ImVec2(100.f, 20.f);
		ImGui::Begin("##PopUpWindow", &show_popup, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImVec2 p = ImGui::GetCursorScreenPos();

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x - 15, p.y - 13), ImVec2(p.x + ImGui::GetWindowWidth(), p.y - 16), ImColor(167, 24, 71, 255), ImColor(58, 31, 87, 255), ImColor(58, 31, 87, 255), ImColor(167, 24, 71, 255));

		long current_time_ms = get_mils();

		ImVec2 text_size = ImGui::CalcTextSize(text);
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() / 2 - text_size.y / 2);
		ImGui::Text(text);

		if (!reverse) {
			if (offset < ImGui::GetWindowWidth())
				offset += (ImGui::GetWindowWidth() + 5) * ((1000.0f / ImGui::GetIO().Framerate) / 100);

			if (offset >= ImGui::GetWindowWidth() && old_time == -1) {
				old_time = current_time_ms;
			}
		}

		if (current_time_ms - old_time >= on_screen_mils && old_time != -1)
			reverse = true;

		if (reverse) {
			if (offset > 0)
				offset -= (ImGui::GetWindowWidth() + 5) * ((1000.0f / ImGui::GetIO().Framerate) / 100);
			if (offset <= 0) {
				offset = 0;
				reverse = false;
				*done = true;
				old_time = -1;
				show_popup = false;
			}
		}

		ImGui::End();
	}
}

void c_menu::run() {
	static int page = 0;

	if (opened) {
		ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.13f, 0.66f, 0.84f);
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.13f, 0.66f, 1.00f);

		ImGui::SetNextWindowSize(ImVec2(800, 387), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Fantail Framework", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar); {
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImColor c = ImColor(32, 114, 247);

			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x, p.y + 70), ImVec2(p.x + ImGui::GetWindowWidth(), p.y - 3), ImColor(167, 24, 71, 255), ImColor(58, 31, 87, 255), ImColor(58, 31, 87, 255), ImColor(167, 24, 71, 255));
			ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x, p.y + 600), ImVec2(p.x + ImGui::GetWindowWidth(), p.y + +30), ImColor(167, 24, 71, 255), ImColor(58, 31, 87, 255), ImColor(58, 31, 87, 255), ImColor(167, 24, 71, 255));
			ImGui::PushFont(font_menu);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7); 
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7); 
			ImGui::Text("  | Fantail Ultimate |                                                                                                          ");
			ImGui::SameLine();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9);
			if (ImGui::ButtonT("AIMBOT", ImVec2(50, 30), page, 0, false, false)) page = 0; ImGui::SameLine(0, 0);
			if (ImGui::ButtonT("PLAYERS", ImVec2(50, 30), page, 1, false, false)) page = 1; ImGui::SameLine(0, 0);
			if (ImGui::ButtonT("VISUALS", ImVec2(50, 30), page, 2, false, false)) page = 2; ImGui::SameLine(0, 0);
			if (ImGui::ButtonT("MISC", ImVec2(50, 30), page, 3, false, false)) page = 3; ImGui::SameLine(0, 0);
			if (ImGui::ButtonT("SKINS", ImVec2(50, 30), page, 4, false, false)) page = 4; ImGui::SameLine(0, 0);
			if (ImGui::ButtonT("CONFIG", ImVec2(50, 30), page, 5, false, false)) page = 5; ImGui::SameLine(0, 0);
			
			ImGui::PopFont();

			ImGui::PushFont(font_menu);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 35);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 222); 

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));

			if (config_system.item.visuals_preview) {
				run_visuals_preview();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			static int test = 0;
			switch (page) {
			case 0:
				ImGui::Columns(2, NULL, false);
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

				//push window color for child
				ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
				//push border color for child
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));
				ImGui::BeginChild("Main", ImVec2(380, 344), true); {
					ImGui::Checkbox("Enable", &config_system.item.aim_enabled);
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.aim_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::Checkbox("Auto Pistol", &config_system.item.aimbot_auto_pistol);
					ImGui::Checkbox("Friendly Fire", &config_system.item.aim_team_check);
					ImGui::Checkbox("Dynamic Fov", &config_system.item.aim_distance_based_fov);
					ImGui::Checkbox("SilentAim", &config_system.item.aim_silent);
					ImGui::Checkbox("Scope Check", &config_system.item.scope_aim);
					ImGui::Checkbox("Smoke Check", &config_system.item.smoke_check);
					ImGui::Combo("HitMode", &config_system.item.aim_mode, "Hitbox\0Nearest"); //todo add custom bone selection - designer
					ImGui::PopStyleColor();
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.aim_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::SliderInt("Kill Delay", &config_system.item.aimbot_delay_after_kill, 0, 350);
					ImGui::PopStyleColor();
					ImGui::Checkbox("Backtrack", &config_system.item.backtrack);
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.backtrack ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::SliderFloat("Amount (ms)", &config_system.item.backtrack_ms, 1.0f, 200.0f, "%.2f");
					ImGui::PopStyleColor();

				} ImGui::EndChild(true);

				ImGui::NextColumn();

				ImGui::BeginChild("Weapon Settings", ImVec2(380, 344), true); {
					if (ImGui::ButtonT("Pistols", ImVec2(50, 30), test, 0, false, ImColor(0, 0, 0))) test = 0; ImGui::SameLine(0, 0);
					if (ImGui::ButtonT("Rifles", ImVec2(50, 30), test, 1, false, false)) test = 1; ImGui::SameLine(0, 0); 
					if (ImGui::ButtonT("Snipers", ImVec2(50, 30), test, 2, false, false)) test = 2; ImGui::SameLine(0, 0); 
					if (ImGui::ButtonT("SMG", ImVec2(50, 30), test, 3, false, false)) test = 3; ImGui::SameLine(0, 0); 
					if (ImGui::ButtonT("Heavy", ImVec2(50, 30), test, 4, false, false)) test = 4; 

					ImGui::PushFont(font_menu);

					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.aim_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					switch (test) {
					case 0:
						if (config_system.item.aim_mode == 0) {
							ImGui::Combo("Hitbox", &config_system.item.aim_bone_pistol, "Head\0Neck\0Chest\0Stomach\0Pelvis");
						}
						ImGui::SliderFloat("FOV", &config_system.item.aim_fov_pistol, 0.0f, 180.0f, "%.2f");
						ImGui::SliderFloat("Smooth", &config_system.item.aim_smooth_pistol, 1.f, 10.f, "%.2f");
						ImGui::SliderFloat("RCS X", &config_system.item.rcs_x_pistol, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("RCS Y", &config_system.item.rcs_y_pistol, 0.0f, 1.0f, "%.2f");
						break;
					case 1:
						if (config_system.item.aim_mode == 0) {
							ImGui::Combo("Hitbox", &config_system.item.aim_bone_rifle, "Head\0Neck\0Chest\0Stomach\0Pelvis");
						}
						ImGui::SliderFloat("FOV", &config_system.item.aim_fov_rifle, 0.0f, 180.0f, "%.2f");
						ImGui::SliderFloat("Smooth", &config_system.item.aim_smooth_rifle, 1.f, 10.f, "%.2f");
						ImGui::SliderFloat("RCS X", &config_system.item.rcs_x_rifle, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("RCS Y", &config_system.item.rcs_y_rifle, 0.0f, 1.0f, "%.2f");
						break;
					case 2:
						if (config_system.item.aim_mode == 0) {
							ImGui::Combo("Hitbox", &config_system.item.aim_bone_sniper, "Head\0Neck\0Chest\0Stomach\0Pelvis");
						}
						ImGui::SliderFloat("FOV", &config_system.item.aim_fov_sniper, 0.0f, 180.0f, "%.2f");
						ImGui::SliderFloat("Smooth", &config_system.item.aim_smooth_sniper, 1.f, 10.f, "%.2f");
						ImGui::SliderFloat("RCS X", &config_system.item.rcs_x_sniper, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("RCS Y", &config_system.item.rcs_y_sniper, 0.0f, 1.0f, "%.2f");
						break;
					case 3:
						if (config_system.item.aim_mode == 0) {
							ImGui::Combo("Hitbox", &config_system.item.aim_bone_smg, "Head\0Neck\0Chest\0Stomach\0Pelvis");
						}

						ImGui::SliderFloat("FOV", &config_system.item.aim_fov_smg, 0.0f, 180.0f, "%.2f");
						ImGui::SliderFloat("Smooth", &config_system.item.aim_smooth_smg, 1.f, 10.f, "%.2f");
						ImGui::SliderFloat("RCS X", &config_system.item.rcs_x_smg, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("RCS Y", &config_system.item.rcs_y_smg, 0.0f, 1.0f, "%.2f");
						break;
					case 4:
						if (config_system.item.aim_mode == 0) {
							ImGui::Combo("Hitbox", &config_system.item.aim_bone_heavy, "Head\0Neck\0Chest\0Stomach\0Pelvis");
						}
						ImGui::SliderFloat("FOV", &config_system.item.aim_fov_heavy, 0.0f, 180.0f, "%.2f");
						ImGui::SliderFloat("Smooth", &config_system.item.aim_smooth_heavy, 1.f, 10.f, "%.2f");
						ImGui::SliderFloat("RCS X", &config_system.item.rcs_x_heavy, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("RCS Y", &config_system.item.rcs_y_heavy, 0.0f, 1.0f, "%.2f");
						break;
					}
					ImGui::PopStyleColor();

					ImGui::PopFont();
				} ImGui::EndChild(true);
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				break;


			case 1:
				ImGui::Columns(2, NULL, false);
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
				ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));

				ImGui::BeginChild("Player", ImVec2(380, 344), true); {
					ImGui::Checkbox("Enable", &config_system.item.visuals_enabled);
					if (config_system.item.visuals_enabled) {
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
						ImGui::Checkbox("Show Team", &config_system.item.visuals_team_check);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
						ImGui::Checkbox("Visible Only", &config_system.item.visuals_visible_only);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
						ImGui::Checkbox("On key", &config_system.item.visuals_on_key);
					}
					ImGui::Checkbox("Name", &config_system.item.player_name);
					ImGui::ColorEdit4("Name Color", config_system.item.clr_name, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Box", &config_system.item.player_box);
					ImGui::ColorEdit4("box Color", config_system.item.clr_box, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Health", &config_system.item.player_health);
					ImGui::Checkbox("Weapon", &config_system.item.player_weapon);
					ImGui::ColorEdit4("Weapon Color", config_system.item.clr_weapon, ImGuiColorEditFlags_NoInputs);

					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.visuals_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					if (ImGui::BeginCombo("Flags", "...", ImVec2(0, 105))) {
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Armor"), &config_system.item.player_flags_armor, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Bomb"), &config_system.item.player_flags_c4, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Flashed"), &config_system.item.player_flags_flashed, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Money"), &config_system.item.player_flags_money, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Scoped"), &config_system.item.player_flags_scoped, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					ImGui::PopStyleColor();
					ImGui::Checkbox("SoundESP", &config_system.item.sound_footstep);
					ImGui::ColorEdit4("SoundESP Color", config_system.item.clr_footstep, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Skeleton", &config_system.item.skeleton);
					ImGui::Checkbox("Preview Visuals", &config_system.item.visuals_preview);
				}
				ImGui::EndChild(true);

				ImGui::NextColumn();

				ImGui::BeginChild("Chams", ImVec2(380, 344), true); {
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.visuals_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::Combo("Chams Type", &config_system.item.vis_chams_type, "Textured\0Flat\0Metallic\0pPulsating");
					ImGui::PopStyleColor();
					ImGui::Checkbox("Enemy", &config_system.item.vis_chams_vis);
					ImGui::ColorEdit4("Enemy Color", config_system.item.clr_chams_vis, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Enemy (Behind Wall)", &config_system.item.vis_chams_invis);
					ImGui::ColorEdit4("Enemy (Behind Wall) Color", config_system.item.clr_chams_invis, ImGuiColorEditFlags_NoInputs);

					ImGui::Checkbox("Team", &config_system.item.vis_chams_vis_teammate);
					ImGui::ColorEdit4("Team color", config_system.item.clr_chams_vis_teammate, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Team (Behind Wall)", &config_system.item.vis_chams_invis_teammate);
					ImGui::ColorEdit4("Teammate (behind wall) color", config_system.item.clr_chams_invis_teammate, ImGuiColorEditFlags_NoInputs);

#ifdef debug_build

					ImGui::Checkbox("Backtrack", &config_system.item.backtrack_chams);
#endif
					ImGui::Checkbox("Smoke check", &config_system.item.vis_chams_smoke_check);
				}
				ImGui::EndChild(true);

				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::Columns();
				break;

				case 2:
					ImGui::Columns(2, NULL, false);
					ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
					ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
					ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));

					ImGui::BeginChild("Effects", ImVec2(380, 344), true); {
						ImGui::Checkbox("Force Crosshair", &config_system.item.force_crosshair);
						ImGui::Checkbox("Modulate Crosshair Color", &config_system.item.crosshair_color);
						ImGui::ColorEdit4("##crosshair color", config_system.item.clr_crosshair, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Modulate Crosshair Outline Color", &config_system.item.crosshair_outline_color);
						ImGui::ColorEdit4("##crosshair outline color", config_system.item.clr_crosshair_outline, ImGuiColorEditFlags_NoInputs);

						ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.visuals_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
						ImGui::SliderInt("Viewmodel FOV", &config_system.item.viewmodel_fov, 0, 135);
						ImGui::SliderInt("FOV", &config_system.item.fov, 0, 60);
						ImGui::PopStyleColor();
						ImGui::Checkbox("Night Mode", &config_system.item.nightmode);
						ImGui::ColorEdit4("Sky Color", config_system.item.clr_sky, ImGuiColorEditFlags_NoInputs);
						if (config_system.item.nightmode) {
							ImGui::SliderInt("Brightness", &config_system.item.nightmode_brightness, 0, 100);
						}

						ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.visuals_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
						if (ImGui::BeginCombo("Removals", "...", ImVec2(0, 105))) {
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Smoke"), &config_system.item.remove_smoke, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Flash"), &config_system.item.reduce_flash, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Sleeves"), &config_system.item.remove_sleeves, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Hands"), &config_system.item.remove_hands, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Scope Overlay"), &config_system.item.remove_scope, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("World", "...", ImVec2(0, 105))) {
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Planted Bomb"), &config_system.item.bomb_planted, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Dropped Weapons"), &config_system.item.dropped_weapons, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Projectiles"), &config_system.item.projectiles, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Misc Entities"), &config_system.item.entity_esp, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
							ImGui::Selectable(("Danger Zone"), &config_system.item.danger_zone_dropped, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::EndCombo();
						}
						ImGui::PopStyleColor();
					}
					ImGui::EndChild(true);

					ImGui::NextColumn();

					ImGui::BeginChild("Glow", ImVec2(380, 344), true); {
						ImGui::Checkbox("Enable", &config_system.item.visuals_glow);
						ImGui::Checkbox("Enemy", &config_system.item.visuals_glow_enemy);
						ImGui::ColorEdit4("Glow Color", config_system.item.clr_glow, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Team", &config_system.item.visuals_glow_team);
						ImGui::ColorEdit4("Glow Color Team", config_system.item.clr_glow_team, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Planted Bomb", &config_system.item.visuals_glow_planted);
						ImGui::ColorEdit4("Glow Color Planted", config_system.item.clr_glow_planted, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Dropped Weapons", &config_system.item.visuals_glow_weapons);
						ImGui::ColorEdit4("Glow Color Weapons", config_system.item.clr_glow_dropped, ImGuiColorEditFlags_NoInputs);

					}
					ImGui::EndChild(true);

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::Columns();
					break;

			case 3:
				ImGui::Columns(2, NULL, false);
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
				ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));

				ImGui::BeginChild("Misc", ImVec2(380, 344), true); {
					ImGui::Checkbox("Enable", &config_system.item.misc_enabled);
					ImGui::Checkbox("Clantag", &config_system.item.clan_tag);
					ImGui::Checkbox("Engine Radar", &config_system.item.radar);

					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.misc_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					if (ImGui::BeginCombo("Logs", "...", ImVec2(0, 65))) {
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Player Hurt"), &config_system.item.logs_player_hurt, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Player Bought"), &config_system.item.logs_player_bought, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 4);
						ImGui::Selectable(("Config System"), &config_system.item.logs_config_system, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::EndCombo();
					}
					ImGui::PopStyleColor();

					ImGui::Checkbox("Hitmarker", &config_system.item.hitmarker);
					if (config_system.item.hitmarker) {
						ImGui::Combo("Hitmarker Sound", &config_system.item.hitmarker_sound, "None\0One\0Two\0Three");
					}
					ImGui::Checkbox("AntiScreenshot", &config_system.item.anti_screenshot);
					ImGui::Checkbox("Spectators", &config_system.item.spectators_list);
					ImGui::Checkbox("Watermark", &config_system.item.watermark);
					ImGui::Checkbox("Disable Post Processing", &config_system.item.disable_post_processing);
					ImGui::Checkbox("Recoil Crosshair", &config_system.item.recoil_crosshair);
					ImGui::Checkbox("Rank Reveal", &config_system.item.rank_reveal);

					ImGui::Checkbox("Viewmodel Offset", &config_system.item.viewmodel_offset);
					if (config_system.item.viewmodel_offset) {
						ImGui::SliderInt("X", &config_system.item.viewmodel_x, -10, 10);
						ImGui::SliderInt("Y", &config_system.item.viewmodel_y, -10, 10);
						ImGui::SliderInt("Z", &config_system.item.viewmodel_z, -10, 10);

					}

					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.misc_enabled ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
					if (ImGui::Button("Dump Steam ID", ImVec2(84, 18))) {
						utilities::dump_steam_id();
					}

					ImGui::SameLine();
					if (ImGui::Button("Hide Name", ImVec2(84, 18))) {
						utilities::change_name("\n\xAD\xAD\xAD");
					}
					ImGui::PopStyleColor();

				}
				ImGui::EndChild(true);
				ImGui::NextColumn();

				ImGui::BeginChild("Movement", ImVec2(380, 344), true); {
					ImGui::Checkbox("Bunny hop", &config_system.item.bunny_hop);
					if (config_system.item.bunny_hop) {
						ImGui::SliderInt("Hitchance", &config_system.item.bunny_hop_hitchance, 0, 100);
						ImGui::SliderInt("Minimum hops", &config_system.item.bunny_hop_minimum_value, 0, 20);
						ImGui::SliderInt("Maximum hops", &config_system.item.bunny_hop_maximum_value, 0, 20);
					}


					ImGui::Checkbox("Edge Jump", &config_system.item.edge_jump);
					if (config_system.item.edge_jump) {
						ImGui::Checkbox("Duck in Air", &config_system.item.edge_jump_duck_in_air);
					}
				}
				ImGui::EndChild(true);
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::Columns();
				break;

			case 4:
				ImGui::Columns(2, NULL, false);
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
				ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));
				ImGui::BeginChild("Main and Knife", ImVec2(380, 173), true); {
					ImGui::Checkbox("Enable SkinChanger", &config_system.item.skinchanger_enable);
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.skinchanger_enable ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::Combo("Model", &config_system.item.knife_model, "Default\0Bayonet\0M9\0Karambit\0Bowie\0Butterfly\0Falchion\0Flip\0Gut\0Huntsman\0Shaddow Daggers\0Navaja\0Stiletto\0Talon\0Orsus");
					ImGui::Combo("Condition", &config_system.item.knife_wear, "Factory New\0Minimal Wear\0Field-Tested\0Well-Worn\0Battle-Scarred");

					ImGui::Combo(("Paintkit"), &config_system.item.paint_kit_vector_index_knife, [](void* data, int idx, const char** out_text) {
						*out_text = parser_skins[idx].name.c_str();
						return true;
					}, nullptr, parser_skins.size(), 10);
					config_system.item.paint_kit_index_knife = parser_skins[config_system.item.paint_kit_vector_index_knife].id;
					ImGui::PopStyleColor();

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
					if (ImGui::Button("Update Skins!", ImVec2(100, 18))) {
						utilities::force_update();
					}

				}
				ImGui::EndChild(true);

				ImGui::PopStyleVar();
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

				ImGui::BeginChild("Gloves", ImVec2(380, 163), true); {
					ImGui::Checkbox("Enable", &config_system.item.glovechanger_enable);
					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.glovechanger_enable ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					ImGui::Combo("Model", &config_system.item.glove_model, "Default\0Blood\0Sport\0Slick\0Leather\0Moto\0Specin\0Hydra");
					ImGui::Combo(("PaintKit"), &config_system.item.paint_kit_vector_index_glove, [](void* data, int idx, const char** out_text) {
						*out_text = parser_gloves[idx].name.c_str();
						return true;
						}, nullptr, parser_gloves.size(), 10);
					config_system.item.paint_kit_index_glove = parser_gloves[config_system.item.paint_kit_vector_index_glove].id;
					ImGui::Combo("Condition", &config_system.item.glove_wear, "Factory New\0Minimal Wear\0Field-Tested\0Well-Worn\0Battle-Scarred");

					ImGui::PopStyleColor();

				}
				ImGui::EndChild(true);

				ImGui::NextColumn();

				ImGui::BeginChild("Weapons", ImVec2(380, 344), true); {
					static int weapons_page = 0;
					if (ImGui::ButtonT("Pistols", ImVec2(50, 30), weapons_page, 0, false, ImColor(0, 0, 0))) weapons_page = 0; ImGui::SameLine(0, 0);
					if (ImGui::ButtonT("Rifles", ImVec2(50, 30), weapons_page, 1, false, false)) weapons_page = 1; ImGui::SameLine(0, 0);
					if (ImGui::ButtonT("Snipers", ImVec2(50, 30), weapons_page, 2, false, false)) weapons_page = 2; ImGui::SameLine(0, 0);
					if (ImGui::ButtonT("SMG", ImVec2(50, 30), weapons_page, 3, false, false)) weapons_page = 3; ImGui::SameLine(0, 0);
					if (ImGui::ButtonT("Heavys", ImVec2(50, 30), weapons_page, 4, false, false)) weapons_page = 4;

					ImGui::PushStyleColor(ImGuiCol_Text, config_system.item.skinchanger_enable ? ImVec4(1.f, 1.f, 1.f, 1) : ImVec4(.6f, .6f, .6f, 1));
					switch (weapons_page) {
					case 0:
						ImGui::Combo(("P2000"), &config_system.item.paint_kit_vector_index_p2000, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_p2000 = parser_skins[config_system.item.paint_kit_vector_index_p2000].id;


						ImGui::Combo(("USP-S"), &config_system.item.paint_kit_vector_index_usp, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_usp = parser_skins[config_system.item.paint_kit_vector_index_usp].id;

						ImGui::Combo(("Glock"), &config_system.item.paint_kit_vector_index_glock, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_glock = parser_skins[config_system.item.paint_kit_vector_index_glock].id;

						ImGui::Combo(("P250"), &config_system.item.paint_kit_vector_index_p250, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_p250 = parser_skins[config_system.item.paint_kit_vector_index_p250].id;

						ImGui::Combo(("Five-Seven"), &config_system.item.paint_kit_vector_index_fiveseven, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_fiveseven = parser_skins[config_system.item.paint_kit_vector_index_fiveseven].id;

						ImGui::Combo(("Tec-9"), &config_system.item.paint_kit_vector_index_tec, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_tec = parser_skins[config_system.item.paint_kit_vector_index_tec].id;

						ImGui::Combo(("CZ75A"), &config_system.item.paint_kit_vector_index_cz, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_cz = parser_skins[config_system.item.paint_kit_vector_index_cz].id;

						ImGui::Combo(("Dual Berretas"), &config_system.item.paint_kit_vector_index_duals, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_duals = parser_skins[config_system.item.paint_kit_vector_index_duals].id;

						ImGui::Combo(("Desert Eagle"), &config_system.item.paint_kit_vector_index_deagle, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_deagle = parser_skins[config_system.item.paint_kit_vector_index_deagle].id;

						ImGui::Combo(("Revolver R8"), &config_system.item.paint_kit_vector_index_revolver, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_revolver = parser_skins[config_system.item.paint_kit_vector_index_revolver].id;

						break;
					case 1:
						ImGui::Combo(("Famas"), &config_system.item.paint_kit_vector_index_famas, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_famas = parser_skins[config_system.item.paint_kit_vector_index_famas].id;

						ImGui::Combo(("Galil"), &config_system.item.paint_kit_vector_index_galil, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_galil = parser_skins[config_system.item.paint_kit_vector_index_galil].id;

						ImGui::Combo(("M4A4"), &config_system.item.paint_kit_vector_index_m4a4, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_m4a4 = parser_skins[config_system.item.paint_kit_vector_index_m4a4].id;

						ImGui::Combo(("M4A1"), &config_system.item.paint_kit_vector_index_m4a1, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_m4a1 = parser_skins[config_system.item.paint_kit_vector_index_m4a1].id;

						ImGui::Combo(("AK47"), &config_system.item.paint_kit_vector_index_ak47, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_ak47 = parser_skins[config_system.item.paint_kit_vector_index_ak47].id;

						ImGui::Combo(("SG553"), &config_system.item.paint_kit_vector_index_sg553, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_sg553 = parser_skins[config_system.item.paint_kit_vector_index_sg553].id;

						ImGui::Combo(("AUG"), &config_system.item.paint_kit_vector_index_aug, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_aug = parser_skins[config_system.item.paint_kit_vector_index_aug].id;

						break;
					case 2:
						ImGui::Combo(("SSG08"), &config_system.item.paint_kit_vector_index_ssg08, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_ssg08 = parser_skins[config_system.item.paint_kit_vector_index_ssg08].id;

						ImGui::Combo(("AWP"), &config_system.item.paint_kit_vector_index_awp, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_awp = parser_skins[config_system.item.paint_kit_vector_index_awp].id;

						ImGui::Combo(("SCAR20"), &config_system.item.paint_kit_vector_index_scar, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_scar = parser_skins[config_system.item.paint_kit_vector_index_scar].id;

						ImGui::Combo(("G3SG1"), &config_system.item.paint_kit_vector_index_g3sg1, [](void* data, int idx, const char** out_text) {
							*out_text = parser_skins[idx].name.c_str();
							return true;
						}, nullptr, parser_skins.size(), 10);
						config_system.item.paint_kit_index_g3sg1 = parser_skins[config_system.item.paint_kit_vector_index_g3sg1].id;

						break;
					case 3:
						break;
					case 4:
						break;

					}
					ImGui::PopStyleColor();
				}
				ImGui::EndChild(true);
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::Columns();
				break;

			case 5:
				ImGui::Columns(2, NULL, false);
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
				ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));
				ImGui::BeginChild("Information", ImVec2(380, 168), true); {
					char buffer[UNLEN + 1];
					DWORD size;
					size = sizeof(buffer);
					GetUserName(buffer, &size);
					char title[UNLEN];
					char ch1[25] = "Welcome, ";
					char* ch = strcat(ch1, buffer);

					ImGui::Text(ch);
					ImGui::Text("Updated: " __DATE__ " / " __TIME__);
					if (std::strstr(GetCommandLineA(), "-Insecure")) {
						ImGui::Text("Insecure mode!");
					}
				}
				ImGui::EndChild(true);

				ImGui::PopStyleVar();
				ImGui::Dummy(ImVec2(0, -2)); ImGui::SameLine();
				ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

				ImGui::BeginChild("Settings", ImVec2(380, 168), true); {
					ImGui::Combo("Keybinds", &config_system.item.keybinds_selection, "Edge Jump\0Aimbot Key\0Visuals Key");

					if (config_system.item.keybinds_selection == 0) {
						ImGui::Hotkey("##edge jump key", &config_system.item.edge_jump_key, ImVec2(100, 20));
					}

					else if (config_system.item.keybinds_selection == 1) {
						ImGui::Hotkey("##aimbot key", &config_system.item.aim_key, ImVec2(100, 20));
					}

					else if (config_system.item.keybinds_selection == 2) {
						ImGui::Hotkey("##visuals key", &config_system.item.visuals_key, ImVec2(100, 20));
					}
				}
				ImGui::EndChild(true);

				ImGui::NextColumn();

				ImGui::BeginChild("Configurations", ImVec2(380, 344), true); {
					constexpr auto& config_items = config_system.get_configs();
					static int current_config = -1;

					if (static_cast<size_t>(current_config) >= config_items.size())
						current_config = -1;

					static char buffer[16];

					if (ImGui::ListBox("", &current_config, [](void* data, int idx, const char** out_text) {
						auto& vector = *static_cast<std::vector<std::string>*>(data);
						*out_text = vector[idx].c_str();
						return true;
						}, &config_items, config_items.size(), 5) && current_config != -1)
						strcpy(buffer, config_items[current_config].c_str());

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
						ImGui::PushID(0);
						ImGui::PushItemWidth(178);
						if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
							if (current_config != -1)
								config_system.rename(current_config, buffer);
						}
						ImGui::PopID();
						ImGui::NextColumn();

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
						if (ImGui::Button(("Create"), ImVec2(85, 20))) {
							config_system.add(buffer);
						}

						ImGui::SameLine();

						if (ImGui::Button(("Reset"), ImVec2(85, 20))) {
							config_system.reset();
						}

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
						if (current_config != -1) {
							if (ImGui::Button(("Load"), ImVec2(85, 20))) {
								config_system.load(current_config);

								load_config = true;

								if (config_system.item.logs_config_system) {
									utilities::console_warning("[config system] ");
									interfaces::console->console_printf(config_items[current_config].c_str());
									interfaces::console->console_printf("[fantail ultimate] loaded. \n");
									std::stringstream ss;
									ss << config_items[current_config].c_str() << "[fantail ultimate] loaded.";
									event_logs.add(ss.str(), color(167, 255, 255, 255));
								}

							}

							ImGui::SameLine();

							if (ImGui::Button(("Save"), ImVec2(85, 20))) {
								config_system.save(current_config);

								save_config = true;

								if (config_system.item.logs_config_system) {
									utilities::console_warning("[config system] ");
									interfaces::console->console_printf(config_items[current_config].c_str());
									interfaces::console->console_printf("[fantail ultimate] Saved. \n");
									std::stringstream ss;
									ss << config_items[current_config].c_str() << "[fantail ultimate] Saved.";
									event_logs.add(ss.str(), color(167, 255, 255, 255));
								}

							}

							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18);
							if (ImGui::Button(("Remove"), ImVec2(85, 20))) {
								config_system.remove(current_config);
							}
						}
				
				}
				ImGui::EndChild(true);
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::Columns();
				break;
			}
		}

		ImGui::PopFont();

		ImGui::End();
	}
}

void c_menu::run_popup() {
	ImGui::PushFont(font_menu);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30 / 255.f, 30 / 255.f, 39 / 255.f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0 / 255.f, 0 / 255.f, 0 / 255.f, 0.1f));

	if (save_config) {
		bool done = false;
		ImGui::begin_popup("Config Saved.", 2000, &done);
		if (done)
			save_config = false;
	}

	if (load_config) {
		bool done = false;
		ImGui::begin_popup("Config Loaded.", 2000, &done);
		if (done)
			load_config = false;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopFont();
}

void c_menu::run_visuals_preview() {
	static std::vector<esp_info_s> info;
	static bool enabled = true;

	ImGui::SetNextWindowSize(ImVec2(235, 400));

	ImGui::Begin("ESP Preview", &enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar); {
		const auto cur_window = ImGui::GetCurrentWindow();
		const ImVec2 w_pos = cur_window->Pos;

		ImVec2 p = ImGui::GetCursorScreenPos();
		ImColor c = ImColor(32, 114, 247);

		//title bar
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x - 20, p.y + 15), ImVec2(p.x + ImGui::GetWindowWidth(), p.y - 40), ImColor(167, 24, 71, 255), ImColor(58, 31, 87, 255), ImColor(58, 31, 87, 255), ImColor(167, 24, 71, 255));

		//draw gradient bellow title bar
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x - 20, p.y + 400), ImVec2(p.x + ImGui::GetWindowWidth(), p.y + 16), ImColor(167, 24, 71, 255), ImColor(58, 31, 87, 255), ImColor(58, 31, 87, 255), ImColor(167, 24, 71, 255));

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9); //góra, dół
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 8); //lewo prawo
		//render cheat name
		ImGui::Text("Visuals Preview");


		if (config_system.item.player_box) {
			cur_window->DrawList->AddRect(ImVec2(w_pos.x + 40, w_pos.y + 60), ImVec2(w_pos.x + 200, w_pos.y + 360), ImGui::GetColorU32(ImGuiCol_Text));
		}

		if (config_system.item.player_health)
			cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + 34, w_pos.y + 60), ImVec2(w_pos.x + 36, w_pos.y + 360), ImGui::GetColorU32(ImVec4(83 / 255.f, 200 / 255.f, 84 / 255.f, 255 / 255.f)));

		if (config_system.item.player_name)
			info.emplace_back(esp_info_s("Name", color(255, 255, 255, 255), CENTER_UP));

		if (config_system.item.player_weapon)
			info.emplace_back(esp_info_s("AWP", color(255, 255, 255, 255), CENTER_DOWN));

		if (config_system.item.player_flags_armor)
			info.emplace_back(esp_info_s("H+K", color(255, 255, 255, 255), RIGHT));


		for (auto render : info) {
			const auto text_size = ImGui::CalcTextSize(render.f_name.c_str());

			auto pos = ImVec2(w_pos.x + 205, w_pos.y + 60);

			if (render.f_position == CENTER_DOWN) {
				pos = ImVec2(w_pos.x + (240 / 2) - text_size.x / 2, pos.y + 315 - text_size.y);
			}
			else if (render.f_position == CENTER_UP) {
				pos = ImVec2(w_pos.x + (240 / 2) - text_size.x / 2, pos.y - 5 - text_size.y);
			}

			cur_window->DrawList->AddText(pos, ImGui::GetColorU32(ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f)), render.f_name.c_str());
		}
	}
	ImGui::End();

	info.clear();
}














































































