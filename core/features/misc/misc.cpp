#include "misc.hpp"

c_misc misc;

void c_misc::remove_smoke() noexcept {
	if (!config_system.item.remove_smoke || !config_system.item.visuals_enabled)
		return;

	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	static auto smoke_count = *reinterpret_cast<uint32_t **>(utilities::pattern_scan(GetModuleHandleA("client_panorama.dll"), "A3 ? ? ? ? 57 8B CB") + 1);

	static std::vector<const char*> smoke_materials = {
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust"
	};

	for (auto material_name : smoke_materials) {
		i_material * smoke = interfaces::material_system->find_material(material_name, TEXTURE_GROUP_OTHER);
		smoke->increment_reference_count();
		smoke->set_material_var_flag(MATERIAL_VAR_WIREFRAME, true);

		*(int*)smoke_count = 0;
	}
}

void c_misc::remove_flash() noexcept {
	if (!config_system.item.reduce_flash || !config_system.item.visuals_enabled)
		return;

	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	if (local_player->flash_duration() > 0.0f)
		local_player->flash_duration() = 0.0f;
}

void c_misc::rank_reveal() noexcept {
	if (!config_system.item.rank_reveal || !config_system.item.misc_enabled)
		return;

	if (GetAsyncKeyState(VK_TAB))
		interfaces::client->dispatch_user_message(cs_um_serverrankrevealall, 0, 0, nullptr);
}

void c_misc::remove_scope() noexcept {
	if (!config_system.item.remove_scope || !config_system.item.visuals_enabled)
		return;

	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	if (local_player && !local_player->is_scoped())
		return;

	int w, h;
	interfaces::engine->get_screen_size(w, h);
	interfaces::surface->set_drawing_color(0, 0, 0, 255);
	interfaces::surface->draw_line(0, h / 2, w, h / 2);
	interfaces::surface->draw_line(w / 2, 0, w / 2, h);
}

void c_misc::spectators() noexcept {
	if (!config_system.item.spectators_list || !config_system.item.misc_enabled)
		return;

	if (!interfaces::engine->is_connected() && !interfaces::engine->is_in_game())
		return;

	int spectator_index = 0;
	int width, height;
	interfaces::engine->get_screen_size(width, height);

	render.draw_text(width - 80, height / 2 - 10, render.name_font, "Spectators List", true, color(255, 255, 255));
	for (int i = 0; i < interfaces::entity_list->get_highest_index(); i++) {
		auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

		if (!local_player)
			return;

		auto entity = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(i));

		if (!entity)
			return;

		player_info_t info;

		if (entity && entity != local_player) {
			interfaces::engine->get_player_info(i, &info);
			if (!entity->is_alive() && !entity->dormant()) {
				auto target = entity->observer_target();

				if (!target)
					return;

				if (target) {
					auto spectator_target = interfaces::entity_list->get_client_entity_handle(target);
					if (spectator_target == local_player) {
						std::string player_name = info.name;
						std::transform(player_name.begin(), player_name.end(), player_name.begin(), ::tolower);
						player_info_t spectator_info;
						interfaces::engine->get_player_info(i, &spectator_info);
						render.draw_text(width - 80, height / 2 + (10 * spectator_index), render.name_font, player_name.c_str(), true, color(255, 255, 255));
						spectator_index++;
					}
				}
			}
		}
	}
}

void c_misc::watermark() noexcept {
	if (!config_system.item.watermark || !config_system.item.misc_enabled)
		return;

	int width, height;
	interfaces::engine->get_screen_size(width, height);

	static int fps, old_tick_count;

	if ((interfaces::globals->tick_count - old_tick_count) > 50) {
		fps = static_cast<int>(1.f / interfaces::globals->frame_time);
		old_tick_count = interfaces::globals->tick_count;
	}
	std::stringstream ss;

	auto net_channel = interfaces::engine->get_net_channel_info();
	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	std::string incoming = local_player ? std::to_string(static_cast<int>(net_channel->get_latency(FLOW_INCOMING) * 1000)) : "0";
	std::string outgoing = local_player ? std::to_string(static_cast<int>(net_channel->get_latency(FLOW_OUTGOING) * 1000)) : "0";

	ss << "fantail.pro | FPS: " << fps << " | Incoming: " << incoming.c_str() << "MS" << " | Outgoing: " << outgoing.c_str() << "MS";

	render.draw_filled_rect(width - 275, 4, 260, 20, color(33, 35, 47, 0));
	render.draw_outline(width - 275, 4, 260, 20, color(30, 30, 41, 0));
	render.draw_text(width - 270, 7, render.watermark_font, ss.str().c_str(), false, color(255, 255, 255, 255));
}

void c_misc::clantag_spammer() noexcept {
	if (!config_system.item.clan_tag || !config_system.item.misc_enabled)
		return;

	static std::string tag = "fantail.pro ";
	static float last_time = 0;

	if (interfaces::globals->cur_time > last_time) {
		std::rotate(std::begin(tag), std::next(std::begin(tag)), std::end(tag));
		utilities::apply_clan_tag(tag.c_str());

		last_time = interfaces::globals->cur_time + 0.9f;
	}

	if (fabs(last_time - interfaces::globals->cur_time) > 1.f)
		last_time = interfaces::globals->cur_time;
}

void c_misc::viewmodel_offset() noexcept {
	if (!config_system.item.viewmodel_offset || !config_system.item.misc_enabled)
		return;

	interfaces::console->get_convar("viewmodel_offset_x")->set_value(config_system.item.viewmodel_x);
	interfaces::console->get_convar("viewmodel_offset_y")->set_value(config_system.item.viewmodel_y);
	interfaces::console->get_convar("viewmodel_offset_z")->set_value(config_system.item.viewmodel_z);
}

void c_misc::disable_post_processing() noexcept {
	if (!config_system.item.misc_enabled)
		return;

	static auto mat_postprocess_enable = interfaces::console->get_convar("mat_postprocess_enable");
	mat_postprocess_enable->set_value(config_system.item.disable_post_processing ? 0 : 1);
}

void c_misc::recoil_crosshair() noexcept {
	if (!config_system.item.misc_enabled)
		return;

	static auto cl_crosshair_recoil = interfaces::console->get_convar("cl_crosshair_recoil");
	cl_crosshair_recoil->set_value(config_system.item.recoil_crosshair ? 1 : 0);
}

void c_misc::force_crosshair() noexcept {
	if (!config_system.item.misc_enabled)
		return;

	auto local_player = reinterpret_cast<player_t*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));

	if (!local_player)
		return;

	static auto weapon_debug_spread_show = interfaces::console->get_convar("weapon_debug_spread_show");

	if (local_player && local_player->health() > 0) {
		weapon_debug_spread_show->set_value(local_player->is_scoped() || !config_system.item.force_crosshair ? 0 : 3);
	}
}
















































































