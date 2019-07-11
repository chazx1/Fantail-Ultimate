#include "visuals.hpp"
#include "../backtrack/backtrack.hpp"
#include "../../../dependencies/common_includes.hpp"

c_visuals visuals;

void c_visuals::run() noexcept {
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!config_system.item.visuals_enabled)
		return;

	if (config_system.item.anti_screenshot && interfaces::engine->is_taking_screenshot())
		return;

	if (!local_player)
		return;

	//player drawing loop
	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

		if (!entity)
			continue;

		if (entity == local_player)
			continue;

		if (entity->health() <= 0)
			continue;

		if (config_system.item.radar)
			entity->spotted() = true;

		if (entity->team() == local_player->team() && !config_system.item.visuals_team_check)
			continue;

		if (!local_player->can_see_player_pos(entity, entity->get_eye_pos()) && config_system.item.visuals_visible_only)
			continue;

		if (config_system.item.visuals_on_key && !GetAsyncKeyState(config_system.item.visuals_key))
			continue;

		const int fade = (int)((6.66666666667f * interfaces::globals->frame_time) * 255);

		auto new_alpha = alpha[i];
		new_alpha += entity->dormant() ? -fade : fade;

		if (new_alpha > (entity->has_gun_game_immunity() ? 130 : 210))
			new_alpha = (entity->has_gun_game_immunity() ? 130 : 210);
		if (new_alpha < config_system.item.player_dormant ? 50 : 0)
			new_alpha = config_system.item.player_dormant ? 50 : 0;

		alpha[i] = new_alpha;

		player_rendering(entity);
		skeleton(entity);
		last_dormant[i] = entity->dormant();
	}

	//non player drawing loop
	for (int i = 0; i < interfaces::entity_list->get_highest_index(); i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

		if (entity && entity != local_player) {
			auto client_class = entity->client_class();
			auto model_name = interfaces::model_info->get_model_name(entity->model());

			if (client_class->class_id == class_ids::cplantedc4) { //this should be fixed in better ways than this - designer
				bomb_esp(entity);
			}

			entity_esp(entity);
			dropped_weapons(entity);
			projectiles(entity);
		}
	}
}

void c_visuals::entity_esp(player_t* entity) noexcept {
	if (!config_system.item.entity_esp)
		return;

	if (!entity)
		return;

	if (entity->dormant())
		return;

	auto model_name = interfaces::model_info->get_model_name(entity->model());
	vec3_t entity_position, entity_origin;
	entity_origin = entity->origin();
	auto client_class = entity->client_class();

	if (!math.world_to_screen(entity_origin, entity_position))
		return;

	if (client_class->class_id == class_ids::cchicken) {
		render.draw_text(entity_position.x, entity_position.y, render.name_font, "chicken", true, color(255, 255, 255));
	}

	else if (strstr(model_name, "dust_soccer_ball001")) {
		render.draw_text(entity_position.x, entity_position.y, render.name_font, "soccer ball", true, color(255, 255, 255));
	}

	else if (client_class->class_id == class_ids::chostage) {
		render.draw_text(entity_position.x, entity_position.y, render.name_font, "hostage", true, color(255, 255, 255));
	}
}

void c_visuals::player_rendering(player_t* entity) noexcept {
	if ((entity->dormant() && alpha[entity->index()] == 0) && !config_system.item.player_dormant)
		return;

	player_info_t info;
	interfaces::engine->get_player_info(entity->index(), &info);

	box bbox;
	if (!get_playerbox(entity, bbox))
		return;

	if (config_system.item.player_box) {
		auto red = config_system.item.clr_box[0] * 255;
		auto green = config_system.item.clr_box[1] * 255;
		auto blue = config_system.item.clr_box[2] * 255;

		render.draw_outline(bbox.x - 1, bbox.y - 1, bbox.w + 2, bbox.h + 2, color(0, 0, 0, 255 + alpha[entity->index()]));
		render.draw_rect(bbox.x, bbox.y, bbox.w, bbox.h, color(red, green, blue, alpha[entity->index()]));
		render.draw_outline(bbox.x + 1, bbox.y + 1, bbox.w - 2, bbox.h - 2, color(0, 0, 0, 255 + alpha[entity->index()]));
	}
	if (config_system.item.player_health) {
		box temp(bbox.x - 5, bbox.y + (bbox.h - bbox.h * (utilities::math::clamp_value<int>(entity->health(), 0, 100.f) / 100.f)), 1, bbox.h * (utilities::math::clamp_value<int>(entity->health(), 0, 100) / 100.f) - (entity->health() >= 100 ? 0 : -1));
		box temp_bg(bbox.x - 5, bbox.y, 1, bbox.h);

		auto health_color = color((255 - entity->health() * 2.55), (entity->health() * 2.55), 0, alpha[entity->index()]);

		if (entity->health() > 100)
			health_color = color(0, 255, 0);

		render.draw_filled_rect(temp_bg.x - 1, temp_bg.y - 1, temp_bg.w + 2, temp_bg.h + 2, color(0, 0, 0, 25 + alpha[entity->index()]));
		render.draw_filled_rect(temp.x, temp.y, temp.w, temp.h, color(health_color));
	}
	if (config_system.item.player_name) {
		auto red = config_system.item.clr_name[0] * 255;
		auto green = config_system.item.clr_name[1] * 255;
		auto blue = config_system.item.clr_name[2] * 255;

		std::string print(info.fakeplayer ? std::string("bot ").append(info.name).c_str() : info.name);
		std::transform(print.begin(), print.end(), print.begin(), ::tolower);

		render.draw_text(bbox.x + (bbox.w / 2), bbox.y - 13, render.name_font, print, true, color(red, green, blue, alpha[entity->index()]));
	}
	{
		std::vector<std::pair<std::string, color>> flags;

		if (config_system.item.player_flags_armor && entity->has_helmet() && entity->armor() > 0)
			flags.push_back(std::pair<std::string, color>("hk", color(255, 255, 255, alpha[entity->index()])));
		else if (config_system.item.player_flags_armor && !entity->has_helmet() && entity->armor() > 0)
			flags.push_back(std::pair<std::string, color>("k", color(255, 255, 255, alpha[entity->index()])));

		if (config_system.item.player_flags_money && entity->money())
			flags.push_back(std::pair<std::string, color>(std::string("$").append(std::to_string(entity->money())), color(120, 180, 10, alpha[entity->index()])));

		if (config_system.item.player_flags_scoped && entity->is_scoped())
			flags.push_back(std::pair<std::string, color>(std::string("zoom"), color(80, 160, 200, alpha[entity->index()])));

		if (config_system.item.player_flags_c4 && entity->has_c4())
			flags.push_back(std::pair<std::string, color>(std::string("bomb"), color(255, 255, 255, alpha[entity->index()])));

		if (config_system.item.player_flags_flashed && entity->is_flashed())
			flags.push_back(std::pair<std::string, color>(std::string("flashed"), color(255, 255, 255, alpha[entity->index()])));

		auto position = 0;
		for (auto text : flags) {
			render.draw_text(bbox.x + bbox.w + 3, bbox.y + position - 2, render.name_font, text.first, false, text.second);
			position += 10;
		}
	}
	if (config_system.item.player_weapon) {
		auto red = config_system.item.clr_weapon[0] * 255;
		auto green = config_system.item.clr_weapon[1] * 255;
		auto blue = config_system.item.clr_weapon[2] * 255;
		auto weapon = entity->active_weapon();

		if (!weapon)
			return;

		std::string names;
		names = clean_item_name(weapon->get_weapon_data()->weapon_name);

		render.draw_text(bbox.x + (bbox.w / 2), bbox.h + bbox.y + 2, render.name_font, names, true, color(red, green, blue, alpha[entity->index()]));
	}
}

void c_visuals::dropped_weapons(player_t* entity) noexcept {
	auto class_id = entity->client_class()->class_id;
	auto model_name = interfaces::model_info->get_model_name(entity->model());
	auto weapon = entity;

	if (!entity)
		return;

	if (!weapon)
		return;

	vec3_t dropped_weapon_position, dropped_weapon_origin;

	dropped_weapon_origin = weapon->origin();

	if (!math.world_to_screen(dropped_weapon_origin, dropped_weapon_position))
		return;

	if (!(entity->origin().x == 0 && entity->origin().y == 0 && entity->origin().z == 0)) { //ghetto fix sorry - designer
		if (config_system.item.dropped_weapons) {
			if (strstr(entity->client_class()->network_name, ("CWeapon"))) {
				std::string data = strstr(entity->client_class()->network_name, ("CWeapon"));
				std::transform(data.begin(), data.end(), data.begin(), ::tolower); //convert dropped weapons names to lowercase, looks cleaner - designer
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, clean_item_name(data), true, color(255, 255, 255));
			}

			if (class_id == class_ids::cak47)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "ak47", true, color(255, 255, 255));

			if (class_id == class_ids::cc4)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "bomb", true, color(255, 255, 255));

			if (class_id == class_ids::cdeagle)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "deagle", true, color(255, 255, 255));

			if (strstr(model_name, "w_defuser"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "defuse kit", true, color(255, 255, 255));
		}

		if (config_system.item.danger_zone_dropped) { 	//no need to create separate func for danger zone shit - designer (also use switch instead of else if)
			if (strstr(model_name, "case_pistol"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "pistol case", true, color(255, 152, 56));

			else if (strstr(model_name, "case_light_weapon"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "light case", true, color(255, 152, 56));

			else if (strstr(model_name, "case_heavy_weapon"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "heavy case", true, color(255, 152, 56));

			else if (strstr(model_name, "case_explosive"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "explosive case", true, color(255, 152, 56));

			else if (strstr(model_name, "case_tools"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "tools case", true, color(255, 152, 56));

			else if (strstr(model_name, "random"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "airdrop", true, color(255, 255, 255));

			else if (strstr(model_name, "dz_armor_helmet"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "armor, helmet", true, color(66, 89, 244));

			else if (strstr(model_name, "dz_helmet"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "helmet", true, color(66, 89, 244));

			else if (strstr(model_name, "dz_armor"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "armor", true, color(66, 89, 244));

			else if (strstr(model_name, "upgrade_tablet"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "tablet upgrade", true, color(255, 255, 255));

			else if (strstr(model_name, "briefcase"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "briefcase", true, color(255, 255, 255));

			else if (strstr(model_name, "parachutepack"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "parachutepack", true, color(255, 255, 255));

			else if (strstr(model_name, "dufflebag"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "cash bag", true, color(120, 180, 10));

			else if (strstr(model_name, "ammobox"))
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "ammobox", true, color(158, 48, 255));

			else if (class_id == class_ids::cdrone)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "drone", true, color(255, 255, 255));

			else if (class_id == class_ids::cphyspropradarjammer)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "radar jammer", true, color(255, 255, 255));

			else if (class_id == class_ids::cdronegun)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "sentry turret", true, color(255, 30, 30));

			else if (class_id == class_ids::cknife)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "knife", true, color(255, 255, 255));

			else if (class_id == class_ids::cmelee)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "melee", true, color(255, 255, 255));

			else if (class_id == class_ids::citemcash)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "cash", true, color(120, 180, 10));

			else if (class_id == class_ids::citem_healthshot)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "health shot", true, color(66, 89, 244));

			else if (class_id == class_ids::ctablet)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "tablet", true, color(255, 255, 255));

			else if (class_id == class_ids::chostage)
				render.draw_text(dropped_weapon_position.x, dropped_weapon_position.y, render.name_font, "hostage", true, color(255, 255, 255));
		}
	}
}

void c_visuals::projectiles(player_t* entity) noexcept {
	if (!config_system.item.projectiles)
		return;

	if (!entity)
		return;

	auto client_class = entity->client_class();
	auto model = entity->model();

	if (!model)
		return;

	if (model) {
		vec3_t grenade_position, grenade_origin;

		auto model = interfaces::model_info->get_studio_model(entity->model());

		if (!model || !strstr(model->name_char_array, "thrown") && !strstr(model->name_char_array, "dropped"))
			return;

		std::string name = model->name_char_array;
		grenade_origin = entity->origin();

		if (!math.world_to_screen(grenade_origin, grenade_position))
			return;

		if (name.find("flashbang") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "flashbang", true, color(255, 255, 255));
		}

		else if (name.find("smokegrenade") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "smoke", true, color(255, 255, 255));

			auto time = interfaces::globals->interval_per_tick * (interfaces::globals->tick_count - entity->smoke_grenade_tick_begin());

			if (!(18 - time < 0)) {
				render.draw_filled_rect(grenade_position.x - 18, grenade_position.y + 13, 36, 3, color(10, 10, 10, 180));
				render.draw_filled_rect(grenade_position.x - 18, grenade_position.y + 13, time * 2, 3, color(167, 24, 71, 255));
			}
		}

		else if (name.find("incendiarygrenade") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "incendiary", true, color(255, 255, 255));
		}

		else if (name.find("molotov") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "molotov", true, color(255, 255, 255));
		}

		else if (name.find("fraggrenade") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "grenade", true, color(255, 255, 255));
		}

		else if (name.find("decoy") != std::string::npos) {
			render.draw_text(grenade_position.x, grenade_position.y, render.name_font, "decoy", true, color(255, 255, 255));
		}
	}
}

void c_visuals::bomb_esp(player_t* entity) noexcept {
	if (!config_system.item.bomb_planted)
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!entity)
		return;

	if (!local_player)
		return;

	auto class_id = entity->client_class()->class_id;
	auto explode_time = entity->c4_blow_time();

	int width, height;
	interfaces::engine->get_screen_size(width, height);

	vec3_t bomb_origin, bomb_position;

	bomb_origin = entity->origin();

	explode_time -= interfaces::globals->interval_per_tick * local_player->get_tick_base();
	if (explode_time <= 0)
		explode_time = 0;

	char buffer[64];
	sprintf_s(buffer, "%.1f", explode_time);

	auto c4_timer = interfaces::console->get_convar("mp_c4timer")->get_int();
	auto value = (explode_time * height) / c4_timer;

	float damage;
	auto distance = local_player->get_eye_pos().distance_to(entity->get_eye_pos());
	auto a = 450.7f, b = 75.68f, c = 789.2f, d = ((distance - b) / c), fl_damage = a * exp(-d * d);
	damage = float((std::max)((int)ceilf(utilities::csgo_armor(fl_damage, local_player->armor())), 0));

	std::string damage_text;
	damage_text += "-";
	damage_text += std::to_string((int)(damage));
	damage_text += "HP";

	if (explode_time <= 10) {
		render.draw_filled_rect(0, 0, 10, value, color(255, 0, 0, 180));
	}
	else {
		render.draw_filled_rect(0, 0, 10, value, color(0, 255, 0, 180));
	}

	render.draw_text(12, value - 11, render.name_font, buffer, false, color(255, 255, 255));

	if (local_player->is_alive()) {
		render.draw_text(12, value - 21, render.name_font, damage_text, false, color(255, 255, 255));
	}

	if (local_player->is_alive() && damage >= local_player->health()) {
		render.draw_text(12, value - 31, render.name_font, "FATAL", false, color(255, 255, 255));
	}

	if (!math.world_to_screen(bomb_origin, bomb_position))
		return;

	render.draw_text(bomb_position.x, bomb_position.y, render.name_font, buffer, true, color(255, 255, 255));
	render.draw_filled_rect(bomb_position.x - c4_timer / 2, bomb_position.y + 13, c4_timer, 3, color(10, 10, 10, 180));
	render.draw_filled_rect(bomb_position.x - c4_timer / 2, bomb_position.y + 13, explode_time, 3, color(167, 24, 71, 255));
}

void c_visuals::chams() noexcept {
	if (!config_system.item.visuals_enabled || (!config_system.item.vis_chams_vis && !config_system.item.vis_chams_invis))
		return;

	for (int i = 1; i <= interfaces::globals->max_clients; i++) {
		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));
		auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

		if (!entity || !entity->is_alive() || entity->dormant() || !local_player)
			continue;

		bool is_teammate = entity->team() == local_player->team();
		bool is_enemy = entity->team() != local_player->team();

		static i_material* mat = nullptr;
		auto textured = interfaces::material_system->find_material("aristois_material", TEXTURE_GROUP_MODEL, true, nullptr);
		auto metalic = interfaces::material_system->find_material("aristois_reflective", TEXTURE_GROUP_MODEL, true, nullptr);
		auto dogtag = interfaces::material_system->find_material("models/inventory_items/dogtags/dogtags_outline", TEXTURE_GROUP_MODEL, true, nullptr);
		auto flat = interfaces::material_system->find_material("debug/debugdrawflat", TEXTURE_GROUP_MODEL, true, nullptr);
		textured->increment_reference_count();  //we need increment_reference_count cuz without it our materialsystem.dll will crash after  map change - designer
		metalic->increment_reference_count();
		flat->increment_reference_count();
		dogtag->increment_reference_count();

		switch (config_system.item.vis_chams_type) {
		case 0:
			mat = textured;
			break;
		case 1:
			mat = flat;
			break;
		case 2:
			mat = metalic;
			break;
		case 3:
			mat = dogtag;
			break;
		}

		if (is_enemy) {
			if (config_system.item.vis_chams_invis) {
				if (utilities::is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head)) && config_system.item.vis_chams_smoke_check)
					return;
				interfaces::render_view->modulate_color(config_system.item.clr_chams_invis);
				interfaces::render_view->set_blend(config_system.item.clr_chams_invis[3]);
				mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);

				interfaces::model_render->override_material(mat);
				entity->draw_model(1, 255);
			}
			if (config_system.item.vis_chams_vis) {
				if (utilities::is_behind_smoke(local_player->get_eye_pos(), entity->get_hitbox_position(entity, hitbox_head)) && config_system.item.vis_chams_smoke_check)
					return;

				interfaces::render_view->modulate_color(config_system.item.clr_chams_vis);
				interfaces::render_view->set_blend(config_system.item.clr_chams_vis[3]);
				mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, false);

				interfaces::model_render->override_material(mat);
				entity->draw_model(1, 255);
			}
		}

		if (is_teammate) {
			if (config_system.item.vis_chams_invis_teammate) {
				interfaces::render_view->modulate_color(config_system.item.clr_chams_invis_teammate);
				interfaces::render_view->set_blend(config_system.item.clr_chams_invis_teammate[3]);
				mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);

				interfaces::model_render->override_material(mat);
				entity->draw_model(1, 255);
			}
			if (config_system.item.vis_chams_vis_teammate) {
				interfaces::render_view->modulate_color(config_system.item.clr_chams_vis_teammate);
				interfaces::render_view->set_blend(config_system.item.clr_chams_vis_teammate[3]);
				mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, false);

				interfaces::model_render->override_material(mat);
				entity->draw_model(1, 255);
			}
		}

		interfaces::model_render->override_material(nullptr);
	}
}

void c_visuals::glow() noexcept {
	if (!config_system.item.visuals_enabled || !config_system.item.visuals_glow)
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	for (size_t i = 0; i < interfaces::glow_manager->size; i++) {
		auto &glow = interfaces::glow_manager->objects[i];

		if (glow.unused())
			continue;

		auto glow_entity = reinterpret_cast<player_t*>(glow.entity);
		auto client_class = glow_entity->client_class();

		if (!glow_entity || glow_entity->dormant())
			continue;

		auto is_enemy = glow_entity->team() != local_player->team();
		auto is_teammate = glow_entity->team() == local_player->team();

		switch (client_class->class_id) {
		case class_ids::ccsplayer:
			if (is_enemy && config_system.item.visuals_glow_enemy) {
				glow.set(config_system.item.clr_glow[0], config_system.item.clr_glow[1], config_system.item.clr_glow[2], config_system.item.clr_glow[3]);
			}

			else if (is_teammate && config_system.item.visuals_glow_team) {
				glow.set(config_system.item.clr_glow_team[0], config_system.item.clr_glow_team[1], config_system.item.clr_glow_team[2], config_system.item.clr_glow_team[3]);
			}
			break;
		case class_ids::cplantedc4:
			if (config_system.item.visuals_glow_planted) {
				glow.set(config_system.item.clr_glow_planted[0], config_system.item.clr_glow_planted[1], config_system.item.clr_glow_planted[2], config_system.item.clr_glow_planted[3]);
			}
			break;
		}

		if (strstr(client_class->network_name, ("CWeapon")) && config_system.item.visuals_glow_weapons) {
			glow.set(config_system.item.clr_glow_dropped[0], config_system.item.clr_glow_dropped[1], config_system.item.clr_glow_dropped[2], config_system.item.clr_glow_dropped[3]);
		}

		else if (client_class->class_id == class_ids::cak47 && config_system.item.visuals_glow_weapons) {
			glow.set(config_system.item.clr_glow_dropped[0], config_system.item.clr_glow_dropped[1], config_system.item.clr_glow_dropped[2], config_system.item.clr_glow_dropped[3]);
		}

		else if (client_class->class_id == class_ids::cc4 && config_system.item.visuals_glow_weapons) {
			glow.set(config_system.item.clr_glow_dropped[0], config_system.item.clr_glow_dropped[1], config_system.item.clr_glow_dropped[2], config_system.item.clr_glow_dropped[3]);
		}

		else if (client_class->class_id == class_ids::cdeagle && config_system.item.visuals_glow_weapons) {
			glow.set(config_system.item.clr_glow_dropped[0], config_system.item.clr_glow_dropped[1], config_system.item.clr_glow_dropped[2], config_system.item.clr_glow_dropped[3]);
		}

	}
}

void c_visuals::skeleton(player_t* entity) noexcept {
	if (!config_system.item.skeleton)
		return;

	auto p_studio_hdr = interfaces::model_info->get_studio_model(entity->model());

	if (!p_studio_hdr)
		return;

	vec3_t v_parent, v_child, s_parent, s_child;

	for (int i = 0; i < p_studio_hdr->bones_count; i++) {
		studio_bone_t* bone = p_studio_hdr->bone(i);

		if (!bone)
			return;

		if (bone && (bone->flags & BONE_USED_BY_HITBOX) && (bone->parent != -1)) {
			v_child = entity->get_bone_position(i);
			v_parent = entity->get_bone_position(bone->parent);

			if (math.world_to_screen(v_parent, s_parent) && math.world_to_screen(v_child, s_child))
				render.draw_line(s_parent[0], s_parent[1], s_child[0], s_child[1], color(255, 255, 255, alpha[entity->index()]));
		}
	}
}

void c_visuals::backtrack_chams(IMatRenderContext* ctx, const draw_model_state_t& state, const model_render_info_t& info) {
	if (!config_system.item.backtrack_chams)
		return;

	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;
}

void c_visuals::viewmodel_modulate(const model_render_info_t& info) {
	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	auto model_name = interfaces::model_info->get_model_name((model_t*)info.model);

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	if (!local_player->is_alive())
		return;

	if (strstr(model_name, "sleeve")) {
		if (config_system.item.remove_sleeves) {
			interfaces::render_view->set_blend(0.f);
		}
	}

	if (strstr(model_name, "arms")) {
		if (config_system.item.remove_hands) {
			interfaces::render_view->set_blend(0.f);
		}
	}
}













































































