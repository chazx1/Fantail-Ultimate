#include <cmath>
#include "aimbot.hpp"
#include "../../../source-sdk/sdk.hpp"
#include "../../../source-sdk/math/vector2d.hpp"
#include "../../../dependencies/common_includes.hpp"
#include "../../features/backtrack/backtrack.hpp"

c_aimbot aimbot;

int c_aimbot::get_nearest_bone(player_t* entity, c_usercmd* user_cmd) noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return false;

	float best_dist = 360.f;
	int aimbone;

	matrix_t matrix[MAXSTUDIOBONES];

	if (!entity->setup_bones(matrix, 128, BONE_USED_BY_HITBOX, 0.0f))
		return -1;

	studio_hdr_t* studio_model = interfaces::model_info->get_studio_model(entity->model());
	if (!studio_model)
		return -1;

	studio_hitbox_set_t* set = studio_model->hitbox_set(entity->hitbox_set());
	if (!set)
		return -1;

	for (int i = 0; i < set->hitbox_count; i++) {
		if (i >= hitbox_max)
			continue;

		studio_box_t* hitbox = set->hitbox(i);

		if (!hitbox)
			continue;

		auto angle = math.calculate_angle(local_player->get_eye_pos(), vec3_t(matrix[hitbox->bone][0][3], matrix[hitbox->bone][1][3], matrix[hitbox->bone][2][3]), user_cmd->viewangles);
		auto this_dist = std::hypotf(angle.x, angle.y);

		if (best_dist > this_dist) {
			best_dist = this_dist;
			aimbone = hitbox->bone;
			continue;
		}
	}
	return aimbone;
}

void c_aimbot::weapon_settings(weapon_t* weapon) noexcept {
	if (!weapon)
		return;

	if (is_pistol(weapon)) {
		switch (config_system.item.aim_bone_pistol) {
		case 0:
			hitbox_id = hitbox_head;
			break;
		case 1:
			hitbox_id = hitbox_neck;
			break;
		case 2:
			hitbox_id = hitbox_chest;
			break;
		case 3:
			hitbox_id = hitbox_stomach;
			break;
		case 4:
			hitbox_id = hitbox_pelvis;
			break;
		}

		aim_smooth = config_system.item.aim_smooth_pistol;
		aim_fov = config_system.item.aim_fov_pistol;
		rcs_x = config_system.item.rcs_x_pistol;
		rcs_y = config_system.item.rcs_y_pistol;
	}
	else if (is_rifle(weapon)) {
		switch (config_system.item.aim_bone_rifle) {
		case 0:
			hitbox_id = hitbox_head;
			break;
		case 1:
			hitbox_id = hitbox_neck;
			break;
		case 2:
			hitbox_id = hitbox_chest;
			break;
		case 3:
			hitbox_id = hitbox_stomach;
			break;
		case 4:
			hitbox_id = hitbox_pelvis;
			break;
		}

		aim_smooth = config_system.item.aim_smooth_rifle;
		aim_fov = config_system.item.aim_fov_rifle;
		rcs_x = config_system.item.rcs_x_rifle;
		rcs_y = config_system.item.rcs_y_rifle;
	}
	else if (is_sniper(weapon)) {
		switch (config_system.item.aim_bone_sniper) {
		case 0:
			hitbox_id = hitbox_head;
			break;
		case 1:
			hitbox_id = hitbox_neck;
			break;
		case 2:
			hitbox_id = hitbox_chest;
			break;
		case 3:
			hitbox_id = hitbox_stomach;
			break;
		case 4:
			hitbox_id = hitbox_pelvis;
			break;
		}

		aim_smooth = config_system.item.aim_smooth_sniper;
		aim_fov = config_system.item.aim_fov_sniper;
		rcs_x = config_system.item.rcs_x_sniper;
		rcs_y = config_system.item.rcs_y_sniper;
	}
	else if (is_heavy(weapon)) {
		switch (config_system.item.aim_bone_heavy) {
		case 0:
			hitbox_id = hitbox_head;
			break;
		case 1:
			hitbox_id = hitbox_neck;
			break;
		case 2:
			hitbox_id = hitbox_chest;
			break;
		case 3:
			hitbox_id = hitbox_stomach;
			break;
		case 4:
			hitbox_id = hitbox_pelvis;
			break;
		}

		aim_smooth = config_system.item.aim_smooth_heavy;
		aim_fov = config_system.item.aim_fov_heavy;
		rcs_x = config_system.item.rcs_x_heavy;
		rcs_y = config_system.item.rcs_y_heavy;
	}
	else if (is_smg(weapon)) {
		switch (config_system.item.aim_bone_smg) {
		case 0:
			hitbox_id = hitbox_head;
			break;
		case 1:
			hitbox_id = hitbox_neck;
			break;
		case 2:
			hitbox_id = hitbox_chest;
			break;
		case 3:
			hitbox_id = hitbox_stomach;
			break;
		case 4:
			hitbox_id = hitbox_pelvis;
			break;
		}

		aim_smooth = config_system.item.aim_smooth_smg;
		aim_fov = config_system.item.aim_fov_smg;
		rcs_x = config_system.item.rcs_x_smg;
		rcs_y = config_system.item.rcs_y_smg;
	}
}

int c_aimbot::find_target(c_usercmd* user_cmd) noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return false;

	auto best_fov = aim_fov;
	auto best_target = 0;

	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		auto entity_bone_pos = entity->get_bone_position(get_nearest_bone(entity, user_cmd));
		auto local_eye_pos = local_player->get_eye_pos();
		auto distance = local_eye_pos.distance_to(entity_bone_pos);

		if (!entity || entity == local_player || entity->dormant() || !entity->is_alive() || entity->has_gun_game_immunity())
			continue;

		angle = math.calculate_angle(local_eye_pos, entity_bone_pos, user_cmd->viewangles);
		auto fov = config_system.item.aim_distance_based_fov ? math.distance_based_fov(distance, math.calculate_angle_alternative(local_eye_pos, entity_bone_pos), user_cmd) : std::hypotf(angle.x, angle.y);
		if (fov < best_fov) {
			best_fov = fov;
			best_target = i;
		}
	}
	return best_target;
}

void c_aimbot::event_player_death(i_game_event* event) noexcept {
	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player || !local_player->is_alive())
		return;

	auto attacker = interfaces::entity_list->get_client_entity(interfaces::engine->get_player_for_user_id(event->get_int("attacker")));

	if (!attacker)
		return;

	if (attacker == local_player)
		kill_delay = interfaces::globals->tick_count + config_system.item.aimbot_delay_after_kill;
}

void c_aimbot::auto_pistol(c_usercmd* user_cmd) {
	if (!config_system.item.aimbot_auto_pistol)
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	auto weapon = local_player->active_weapon();

	if (!weapon)
		return;

	static bool was_firing = false;

	if (aimbot.is_pistol(weapon) && weapon->item_definition_index() != WEAPON_REVOLVER) {
		if (user_cmd->buttons & in_attack && !aimbot.is_knife(weapon) && !aimbot.is_grenade(weapon)) {
			if (was_firing) {
				user_cmd->buttons &= ~in_attack;
			}
		}

		was_firing = user_cmd->buttons & in_attack ? true : false;
	}
}

void c_aimbot::rcs_standalone(c_usercmd* user_cmd) noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	if (!local_player)
		return;

	static vec3_t old_punch = { 0.0f, 0.0f, 0.0f };
	auto recoil_scale = interfaces::console->get_convar("weapon_recoil_scale");
	auto aim_punch = local_player->aim_punch_angle() * recoil_scale->get_float();

	aim_punch.x *= rcs_x;
	aim_punch.y *= rcs_y;

	auto rcs = user_cmd->viewangles += (old_punch - aim_punch);
	interfaces::engine->set_view_angles(rcs);

	old_punch = aim_punch;
}

void c_aimbot::run(c_usercmd* user_cmd) noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	if (kill_delay >= interfaces::globals->tick_count)
		return;

	auto weapon = local_player->active_weapon();
	weapon_settings(weapon);
	auto_pistol(user_cmd);
	rcs_standalone(user_cmd);

	if (config_system.item.aim_enabled && user_cmd->buttons & in_attack || GetAsyncKeyState(config_system.item.aim_key)) {
		if (auto target = find_target(user_cmd)) {
			auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(target));

			if (!weapon || !weapon->clip1_count())
				return;

			if (!local_player->can_see_player_pos(entity, entity->get_eye_pos()))
				return;

			if (!config_system.item.aim_team_check && entity->team() == local_player->team())
				return;

			if (!config_system.item.smoke_check && utilities::is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head)))
				return;

			if (is_knife(weapon) || is_grenade(weapon)|| is_bomb(weapon))
				return;

			if (is_sniper(weapon) && !local_player->is_scoped() && !config_system.item.scope_aim)
				return;

			switch (config_system.item.aim_mode) {
			case 0:
				angle = math.calculate_angle(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_id), user_cmd->viewangles);
				break;
			case 1:
				angle = math.calculate_angle(local_player->get_eye_pos(), entity->get_bone_position(get_nearest_bone(entity, user_cmd)), user_cmd->viewangles);
				break;
			}
			

			angle /= aim_smooth;
			user_cmd->viewangles += angle;

			if (!config_system.item.aim_silent) {
				interfaces::engine->set_view_angles(user_cmd->viewangles);
			}
		}
	}
} 




















































































