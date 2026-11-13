// Member offsets for the reduced active client schema.
#pragma once

namespace sdk {
namespace offsets {
namespace schema {

namespace game_time_t {
	constexpr auto value = 0x0;
}

namespace cnetwork_view_offset_vector {
	constexpr auto vec_x = 0x10;
	constexpr auto vec_y = 0x18;
	constexpr auto vec_z = 0x20;
}

namespace client {

namespace centity_instance
				{
					constexpr auto isz_private_vscripts = 0x8;
					constexpr auto entity_ptr = 0x10;
					constexpr auto cscript_component_ptr = 0x28;
				}

namespace base_entity
				{
					constexpr auto cbody_component_ptr = 0x30;
					constexpr auto network_transmit_component = 0x38;
					constexpr auto last_think_tick = 0x328;
					constexpr auto game_scene_node_ptr = 0x330;
					constexpr auto render_component_ptr = 0x338;
					constexpr auto collision_ptr = 0x340;
					constexpr auto max_health = 0x348;
					constexpr auto health = 0x34c;
					constexpr auto life_state = 0x354;
					constexpr auto takes_damage = 0x355;
					constexpr auto take_damage_flags = 0x358;
					constexpr auto ub_interpolation_frame = 0x361;
					constexpr auto h_scene_object_controller = 0x364;
					constexpr auto no_interpolation_tick = 0x368;
					constexpr auto visibility_no_interpolation_tick = 0x36c;
					constexpr auto proxy_random_value = 0x370;
					constexpr auto eflags = 0x374;
					constexpr auto water_type = 0x378;
					constexpr auto interpolate_even_with_no_model = 0x379;
					constexpr auto prediction_eligible = 0x37a;
					constexpr auto apply_layer_match_idto_model = 0x37b;
					constexpr auto tok_layer_match_id = 0x37c;
					constexpr auto subclass_id = 0x380;
					constexpr auto simulation_tick = 0x390;
					constexpr auto current_think_context = 0x394;
					constexpr auto a_think_functions = 0x398;
					constexpr auto anim_time = 0x3b4;
					constexpr auto simulation_time = 0x3b8;
					constexpr auto scene_object_override_flags = 0x3bc;
					constexpr auto has_successfully_interpolated = 0x3bd;
					constexpr auto has_added_vars_to_interpolation = 0x3be;
					constexpr auto render_even_when_not_successfully_interpolated = 0x3bf;
					constexpr auto interpolation_latch_dirty_flags_arr = 0x3c0;
					constexpr auto list_entry_arr = 0x3c8;
					constexpr auto create_time = 0x3e0;
					constexpr auto speed = 0x3b8;
					constexpr auto ent_client_flags = 0x3e4;
					constexpr auto client_side_ragdoll = 0x3e6;
					constexpr auto team_num = 0x3e7;
					constexpr auto spawnflags = 0x3e8;
					constexpr auto next_think_tick = 0x3ec;
					constexpr auto flags = 0x3f4;
					constexpr auto vec_abs_velocity = 0x3f8;
					constexpr auto vec_velocity = 0x430;
					constexpr auto vec_base_velocity = 0x510;
					constexpr auto h_effect_entity = 0x51c;
					constexpr auto h_owner_entity = 0x520;
					constexpr auto move_collide = 0x524;
					constexpr auto move_type = 0x525;
					constexpr auto water_level = 0x528;
					constexpr auto effects = 0x52c;
					constexpr auto h_ground_entity = 0x530;
					constexpr auto friction = 0x538;
					constexpr auto elasticity = 0x53c;
					constexpr auto gravity_scale = 0x540;
					constexpr auto time_scale = 0x544;
					constexpr auto simulated_every_tick = 0x43c;
					constexpr auto animated_every_tick = 0x548;
					constexpr auto nav_ignore_until_time = 0x54c;
					constexpr auto h_think = 0x550;
					constexpr auto bbox_vis_flags = 0x560;
					constexpr auto predictable = 0x569;
					constexpr auto render_with_view_models = 0x56a;
					constexpr auto split_user_player_prediction_slot = 0x454;
					constexpr auto first_predictable_command = 0x56c;
					constexpr auto last_predictable_command = 0x570;
					constexpr auto h_old_move_parent = 0x574;
					constexpr auto particles = 0x578;
					constexpr auto vec_predicted_script_floats = 0x490;
					constexpr auto vec_predicted_script_float_ids = 0x4a8;
					constexpr auto next_script_var_record_id = 0x4d8;
					constexpr auto vec_ang_velocity = 0x5a8;
					constexpr auto data_change_event_ref = 0x5b4;
					constexpr auto dependencies = 0x5b8;
					constexpr auto creation_tick = 0x5d0;
					constexpr auto anim_time_changed = 0x5e1;
					constexpr auto simulation_time_changed = 0x5e2;
					constexpr auto s_unique_hammer_id = 0x5f0;
				}

namespace econ_item_view
				{
					constexpr auto inventory_image_rgba_requested = 0x60;
					constexpr auto inventory_image_tried_cache = 0x61;
					constexpr auto inventory_image_rgba_width = 0x80;
					constexpr auto inventory_image_rgba_height = 0x84;
					constexpr auto current_load_cached_file_name_arr = 0x88;
					constexpr auto restore_custom_material_after_precache = 0x1b8;
					constexpr auto item_definition_index = 0x1ba;
					constexpr auto entity_quality = 0x1bc;
					constexpr auto entity_level = 0x1c0;
					constexpr auto item_id = 0x1c8;
					constexpr auto item_idhigh = 0x1d0;
					constexpr auto item_idlow = 0x1d4;
					constexpr auto account_id = 0x1d8;
					constexpr auto inventory_position = 0x1dc;
					constexpr auto initialized = 0x1e8;
					constexpr auto is_store_item = 0x1ea;
					constexpr auto is_trade_item = 0x1eb;
					constexpr auto entity_quantity = 0x1ec;
					constexpr auto rarity_override = 0x1f0;
					constexpr auto quality_override = 0x1f4;
					constexpr auto un_client_flags = 0x1fd;
					constexpr auto un_override_style = 0x1f9;
					constexpr auto attribute_list = 0x208;
					constexpr auto networked_dynamic_attributes = 0x280;
					constexpr auto custom_name_arr = 0x2f8;
					constexpr auto custom_name_override_arr = 0x399;
					constexpr auto initialized_tags = 0x5a8;
				}

namespace cbase_player_controller
				{
					constexpr auto final_predicted_tick = 0x548;
					constexpr auto command_context = 0x608;
					constexpr auto in_buttons_which_are_toggles = 0x6b0;
					constexpr auto tick_base = 0x6b8;
					constexpr auto h_pawn = 0x6bc;
					constexpr auto h_predicted_pawn = 0x6c4;
					constexpr auto split_screen_slot = 0x6cc;
					constexpr auto h_split_owner = 0x6d0;
					constexpr auto h_split_screen_players = 0x6d8;
					constexpr auto is_hltv = 0x6f0;
					constexpr auto connected = 0x6f4;
					constexpr auto isz_player_name_arr = 0x6fc;
					constexpr auto steam_id = 0x788;
					constexpr auto is_local_player_controller = 0x790;
					constexpr auto desired_fov = 0x794;
				}

namespace base_model_entity
				{
					constexpr auto crender_component_ptr = 0xaf8;
					constexpr auto chitbox_component = 0xb00;
					constexpr auto init_model_effects = 0xc78;
					constexpr auto is_static_prop = 0xa61;
					constexpr auto last_add_decal = 0xa64;
					constexpr auto decals_added = 0xa68;
					constexpr auto old_health = 0xc7c;
					constexpr auto render_mode = 0xc80;
					constexpr auto render_fx = 0xc81;
					constexpr auto allow_fade_in_view = 0xc82;
					constexpr auto clr_render = 0xca0;
					constexpr auto vec_render_attributes = 0xca8;
					constexpr auto render_to_cubemaps = 0xd28;
					constexpr auto collision = 0xd30;
					constexpr auto glow = 0xde8;
					constexpr auto glow_backface_mult = 0xe40;
					constexpr auto fade_min_dist = 0xe44;
					constexpr auto fade_max_dist = 0xe48;
					constexpr auto fade_scale = 0xe4c;
					constexpr auto shadow_strength = 0xe50;
					constexpr auto object_culling = 0xe54;
					constexpr auto add_decal = 0xc08;
					constexpr auto v_decal_position = 0xc0c;
					constexpr auto v_decal_forward_axis = 0xc18;
					constexpr auto decal_heal_blood_rate = 0xc24;
					constexpr auto decal_heal_height_rate = 0xc28;
					constexpr auto config_entities_to_propagate_material_decals_to = 0xc30;
					constexpr auto vec_view_offset = 0xf60;
					constexpr auto client_alpha_property_ptr = 0x1040;
					constexpr auto client_override_tint = 0x1048;
					constexpr auto use_client_override_tint = 0x104c;
				}

namespace ccsplayer_controller
				{
					constexpr auto in_game_money_services_ptr = 0x818;
					constexpr auto inventory_services_ptr = 0x820;
					constexpr auto action_tracking_services_ptr = 0x828;
					constexpr auto damage_services_ptr = 0x830;
					constexpr auto ping = 0x838;
					constexpr auto has_communication_abuse_mute = 0x83c;
					constexpr auto crosshair_codes = 0x848;
					constexpr auto pending_team_num = 0x850;
					constexpr auto force_team_time = 0x854;
					constexpr auto comp_teammate_color = 0x858;
					constexpr auto ever_played_on_team = 0x85c;
					constexpr auto previous_force_join_team_time = 0x860;
					constexpr auto clan = 0x868;
					constexpr auto s_sanitized_player_name = 0x878;
					constexpr auto coaching_team = 0x888;
					constexpr auto player_dominated = 0x890;
					constexpr auto player_dominating_me = 0x898;
					constexpr auto competitive_ranking = 0x8a0;
					constexpr auto competitive_wins = 0x8a4;
					constexpr auto competitive_rank_type = 0x8a8;
					constexpr auto competitive_ranking_predicted_win = 0x8ac;
					constexpr auto competitive_ranking_predicted_loss = 0x8b0;
					constexpr auto competitive_ranking_predicted_tie = 0x8b4;
					constexpr auto end_match_next_map_vote = 0x8b8;
					constexpr auto un_active_quest_id = 0x8bc;
					constexpr auto quest_progress_reason = 0x8c4;
					constexpr auto un_player_tv_control_flags = 0x8c8;
					constexpr auto draft_index = 0x8f8;
					constexpr auto ms_queued_mode_disconnection_timestamp = 0x8fc;
					constexpr auto ui_abandon_recorded_reason = 0x900;
					constexpr auto cannot_be_kicked = 0x908;
					constexpr auto ever_fully_connected = 0x909;
					constexpr auto abandon_allows_surrender = 0x90a;
					constexpr auto abandon_offers_instant_surrender = 0x90b;
					constexpr auto disconnection1_min_warning_printed = 0x90c;
					constexpr auto score_reported = 0x90d;
					constexpr auto disconnection_tick = 0x910;
					constexpr auto controlling_bot = 0x920;
					constexpr auto has_controlled_bot_this_round = 0x921;
					constexpr auto has_been_controlled_by_player_this_round = 0x922;
					constexpr auto bots_controlled_this_round = 0x924;
					constexpr auto can_control_observed_bot = 0x928;
					constexpr auto h_player_pawn = 0x92c;
					constexpr auto h_observer_pawn = 0x930;
					constexpr auto pawn_is_alive = 0x934;
					constexpr auto pawn_health = 0x938;
					constexpr auto pawn_armor = 0x93c;
					constexpr auto pawn_has_defuser = 0x940;
					constexpr auto pawn_has_helmet = 0x941;
					constexpr auto pawn_character_def_index = 0x942;
					constexpr auto pawn_lifetime_start = 0x944;
					constexpr auto pawn_lifetime_end = 0x948;
					constexpr auto pawn_bot_difficulty = 0x94c;
					constexpr auto h_original_controller_of_current_pawn = 0x950;
					constexpr auto score = 0x954;
					constexpr auto vec_kills = 0x818;
					constexpr auto mvps = 0x970;
					constexpr auto is_player_name_dirty = 0x974;
				}

namespace cbase_anim_graph
				{
					constexpr auto initially_populate_interp_history = 0x8f0;
					constexpr auto should_animate_during_gameplay_pause = 0xcc1;
					constexpr auto suppress_anim_event_sounds = 0xcc3;
					constexpr auto anim_graph_update_enabled = 0x938;
					constexpr auto max_slope_distance = 0xcd4;
					constexpr auto v_last_slope_check_pos = 0xcd8;
					constexpr auto vec_force = 0x93c;
					constexpr auto force_bone = 0x948;
					constexpr auto clientside_ragdoll_ptr = 0x334;
					constexpr auto built_ragdoll = 0xd00;
					constexpr auto ragdoll_pose_ptr = 0x960;
					constexpr auto client_ragdoll = 0xd20;
					constexpr auto has_animated_material_attributes = 0xd30;
				}

namespace base_flex
				{
					constexpr auto flex_weight = 0xe90;
					constexpr auto v_look_target_position = 0xea8;
					constexpr auto blinktoggle = 0xec0;
					constexpr auto last_flex_update_frame_count = 0xf20;
					constexpr auto cached_view_target = 0xf24;
					constexpr auto next_scene_event_id = 0xf30;
					constexpr auto blink = 0xf34;
					constexpr auto blinktime = 0xf38;
					constexpr auto prevblinktoggle = 0xf3c;
					constexpr auto jaw_open = 0xf40;
					constexpr auto jaw_open_amount = 0xf44;
					constexpr auto blink_amount = 0xf48;
					constexpr auto mouth_attachment = 0xf4c;
					constexpr auto eye_attachment = 0xf4d;
					constexpr auto reset_flex_weights_on_model_change = 0xf4e;
					constexpr auto eye_occlusion_renderer_bone = 0xf68;
					constexpr auto m_eye_occlusion_renderer_camera_to_bone_transform = 0xf6c;
					constexpr auto v_eye_occlusion_renderer_half_extent = 0xf9c;
					constexpr auto phoneme_classes_arr = 0xfb8;
				}

namespace base_combat_character
				{
					constexpr auto h_my_wearables = 0x1268;
					constexpr auto blood_color = 0x1030;
					constexpr auto left_foot_attachment = 0x1280;
					constexpr auto right_foot_attachment = 0x1281;
					constexpr auto water_wake_mode = 0x1284;
					constexpr auto water_world_z = 0x1288;
					constexpr auto water_next_trace_time = 0x128c;
					constexpr auto field_of_view = 0x1044;
				}

namespace base_player_pawn
				{
					constexpr auto weapon_services_ptr = 0x12f0;
					constexpr auto item_services_ptr = 0x12f8;
					constexpr auto autoaim_services_ptr = 0x1300;
					constexpr auto observer_services_ptr = 0x1308;
					constexpr auto water_services_ptr = 0x1310;
					constexpr auto use_services_ptr = 0x1318;
					constexpr auto flashlight_services_ptr = 0x1320;
					constexpr auto camera_services_ptr = 0x1328;
					constexpr auto movement_services_ptr = 0x1330;
					constexpr auto server_view_angle_changes = 0x1340;
					constexpr auto highest_consumed_server_view_angle_change_index = 0x1148;
					constexpr auto v_angle = 0x13a8;
					constexpr auto v_angle_previous = 0x13b4;
					constexpr auto hide_hud = 0x13c0;
					constexpr auto skybox3d = 0x13c8;
					constexpr auto death_time = 0x1458;
					constexpr auto vec_prediction_error = 0x1460;
					constexpr auto prediction_error_time = 0x146c;
					constexpr auto vec_last_camera_setup_local_origin = 0x148c;
					constexpr auto last_camera_setup_time = 0x1498;
					constexpr auto fovsensitivity_adjust = 0x149c;
					constexpr auto mouse_sensitivity = 0x14a0;
					constexpr auto v_old_origin = 0x14a4;
					constexpr auto old_simulation_time = 0x14b0;
					constexpr auto last_executed_command_number = 0x14b4;
					constexpr auto last_executed_command_tick = 0x14b8;
					constexpr auto h_controller = 0x14bc;
					constexpr auto is_swapping_to_predictable_controller = 0x14c4;
				}

namespace csplayer_pawn_base
				{
					constexpr auto ping_services_ptr = 0x14d8;
					constexpr auto view_model_services_ptr = 0x1270;
					constexpr auto rendering_clip_plane_arr = 0x1278;
					constexpr auto last_clip_plane_setup_frame = 0x1288;
					constexpr auto vec_last_clip_camera_pos = 0x128c;
					constexpr auto vec_last_clip_camera_forward = 0x1298;
					constexpr auto clip_hit_static_world = 0x12a4;
					constexpr auto cached_plane_is_valid = 0x12a5;
					constexpr auto clipping_weapon_ptr = 0x12a8;
					constexpr auto previous_player_state = 0x14e0;
					constexpr auto last_collision_ceiling = 0x12b4;
					constexpr auto last_collision_ceiling_change_time = 0x12b8;
					constexpr auto grenade_parameter_stash_time = 0x12d8;
					constexpr auto grenade_parameters_stashed = 0x12dc;
					constexpr auto ang_stashed_shoot_angles = 0x12e0;
					constexpr auto vec_stashed_grenade_throw_position = 0x12ec;
					constexpr auto vec_stashed_velocity = 0x12f8;
					constexpr auto ang_shoot_angle_history_arr = 0x1304;
					constexpr auto vec_throw_position_history_arr = 0x131c;
					constexpr auto vec_velocity_history_arr = 0x1334;
					constexpr auto third_person_heading = 0x1350;
					constexpr auto slope_drop_offset = 0x1368;
					constexpr auto slope_drop_height = 0x1378;
					constexpr auto v_head_constraint_offset = 0x1388;
					constexpr auto is_scoped = 0x13a0;
					constexpr auto is_walking = 0x13a1;
					constexpr auto resume_zoom = 0x13a2;
					constexpr auto player_state = 0x14e4;
					constexpr auto is_defusing = 0x13a8;
					constexpr auto is_grabbing_hostage = 0x13a9;
					constexpr auto blocking_use_action_in_progress = 0x13ac;
					constexpr auto is_rescuing = 0x13b0;
					constexpr auto immune_to_gun_game_damage_time = 0x13b4;
					constexpr auto immune_to_gun_game_damage_time_last = 0x13b8;
					constexpr auto gun_game_immunity = 0x13bc;
					constexpr auto has_moved_since_spawn = 0x14e8;
					constexpr auto molotov_use_time = 0x13c0;
					constexpr auto molotov_damage_time = 0x13c4;
					constexpr auto which_bomb_zone = 0x13c8;
					constexpr auto in_no_defuse_area = 0x13cc;
					constexpr auto throw_grenade_counter = 0x13d0;
					constexpr auto wait_for_no_attack = 0x13d4;
					constexpr auto guardian_too_far_dist_frac = 0x13d8;
					constexpr auto detected_by_enemy_sensor_time = 0x13dc;
					constexpr auto next_guardian_too_far_warning = 0x13e0;
					constexpr auto suppress_guardian_too_far_warning_audio = 0x13e4;
					constexpr auto killed_by_taser = 0x13e5;
					constexpr auto move_state = 0x13e8;
					constexpr auto can_move_during_freeze_period = 0x13ec;
					constexpr auto lower_body_yaw_target = 0x13f0;
					constexpr auto strafing = 0x13f4;
					constexpr auto last_spawn_time_index = 0x14ec;
					constexpr auto emit_sound_time = 0x13fc;
					constexpr auto addon_bits = 0x1400;
					constexpr auto primary_addon = 0x1404;
					constexpr auto secondary_addon = 0x1408;
					constexpr auto progress_bar_duration = 0x14f0;
					constexpr auto progress_bar_start_time = 0x14f4;
					constexpr auto direction = 0x1414;
					constexpr auto shots_fired = 0x1418;
					constexpr auto night_vision_on = 0x141c;
					constexpr auto has_night_vision = 0x141d;
					constexpr auto velocity_modifier = 0x1420;
					constexpr auto hit_heading = 0x1424;
					constexpr auto hit_body_part = 0x1428;
					constexpr auto start_account = 0x142c;
					constexpr auto vec_intro_start_eye_position = 0x1430;
					constexpr auto vec_intro_start_player_forward = 0x143c;
					constexpr auto client_death_time = 0x14f8;
					constexpr auto night_vision_alpha = 0x144c;
					constexpr auto screen_tear_frame_captured = 0x1450;
					constexpr auto flash_bang_time = 0x14fc;
					constexpr auto flash_screenshot_alpha = 0x1500;
					constexpr auto flash_overlay_alpha = 0x1504;
					constexpr auto flash_build_up = 0x1508;
					constexpr auto flash_dsp_has_been_cleared = 0x1509;
					constexpr auto flash_screenshot_has_been_grabbed = 0x150a;
					constexpr auto flash_max_alpha = 0x150c;
					constexpr auto flash_duration = 0x1510;
					constexpr auto last_standing_pos = 0x146c;
					constexpr auto vec_last_muzzle_flash_pos = 0x1478;
					constexpr auto ang_last_muzzle_flash_angle = 0x1484;
					constexpr auto h_muzzle_flash_shape = 0x1490;
					constexpr auto health_bar_render_mask_index = 0x1494;
					constexpr auto health_fade_value = 0x1498;
					constexpr auto health_fade_alpha = 0x149c;
					constexpr auto my_collision_group = 0x14a0;
					constexpr auto ignore_ladder_jump_time = 0x14a4;
					constexpr auto ladder_surpression_timer = 0x14a8;
					constexpr auto last_ladder_normal = 0x14c0;
					constexpr auto last_ladder_pos = 0x14cc;
					constexpr auto death_ccweight = 0x14e0;
					constexpr auto old_is_scoped = 0x14e4;
					constexpr auto prev_round_end_time = 0x14e8;
					constexpr auto prev_match_end_time = 0x14ec;
					constexpr auto un_current_equipment_value = 0x14f0;
					constexpr auto un_round_start_equipment_value = 0x14f2;
					constexpr auto un_freezetime_end_equipment_value = 0x14f4;
					constexpr auto vec_third_person_view_position_override = 0x14f8;
					constexpr auto heavy_assault_suit_cooldown_remaining = 0x1504;
					constexpr auto armor_value = 0x1508;
					constexpr auto ang_eye_angles = 0x1510;
					constexpr auto next_think_push_away = 0x151c;
					constexpr auto should_autobuy_dmweapons = 0x152c;
					constexpr auto should_autobuy_now = 0x152d;
					constexpr auto hud_mini_score_hidden = 0x152e;
					constexpr auto hud_radar_hidden = 0x152f;
					constexpr auto last_killer_index = 0x1530;
					constexpr auto last_concurrent_killed = 0x1534;
					constexpr auto death_cam_music = 0x1538;
					constexpr auto ident_index = 0x153c;
					constexpr auto delay_target_idtimer = 0x1540;
					constexpr auto targeted_weapon_ent_index = 0x1558;
					constexpr auto old_ident_index = 0x155c;
					constexpr auto hold_target_idtimer = 0x1560;
					constexpr auto current_music_start_time = 0x1524;
					constexpr auto music_round_start_time = 0x1528;
					constexpr auto defer_start_music_on_warmup = 0x152c;
					constexpr auto cycle_latch = 0x1588;
					constexpr auto server_intended_cycle = 0x158c;
					constexpr auto vec_player_patch_econ_indices_arr = 0x1590;
					constexpr auto hide_target_id = 0x15ac;
					constexpr auto next_taser_shake_time = 0x15b0;
					constexpr auto first_taser_shake_time = 0x15b4;
					constexpr auto last_smoke_overlay_alpha = 0x1530;
					constexpr auto v_last_smoke_overlay_color = 0x1538;
					constexpr auto player_smoked_fx = 0x15c8;
					constexpr auto next_mag_drop_time = 0x15cc;
					constexpr auto last_mag_drop_attachment_index = 0x15d0;
					constexpr auto vec_bullet_hit_models = 0x15d8;
					constexpr auto vec_pickup_model_slerpers = 0x15f0;
					constexpr auto vec_last_alive_local_velocity = 0x1608;
					constexpr auto entity_spotted_state = 0x1630;
					constexpr auto survival_team_number = 0x1648;
					constexpr auto guardian_should_spray_custom_xmark = 0x164c;
					constexpr auto has_death_info = 0x164d;
					constexpr auto death_info_time = 0x1650;
					constexpr auto vec_death_info_origin = 0x1654;
					constexpr auto killed_by_headshot = 0x1660;
					constexpr auto h_original_controller = 0x1560;
				}

namespace csplayer_pawn
				{
					constexpr auto bullet_services_ptr = 0x1570;
					constexpr auto hostage_services_ptr = 0x1578;
					constexpr auto buy_services_ptr = 0x1580;
					constexpr auto glow_services_ptr = 0x1588;
					constexpr auto action_tracking_services_ptr = 0x1590;
					constexpr auto health_shot_boost_expiration_time = 0x15a8;
					constexpr auto last_fired_weapon_time = 0x15ac;
					constexpr auto has_female_voice = 0x15b0;
					constexpr auto landseconds = 0x16e4;
					constexpr auto old_fall_velocity = 0x15b8;
					constexpr auto last_place_name_arr = 0x15bc;
					constexpr auto prev_defuser = 0x15ce;
					constexpr auto prev_helmet = 0x15cf;
					constexpr auto prev_armor_val = 0x15d0;
					constexpr auto prev_grenade_ammo_count = 0x15d4;
					constexpr auto un_previous_weapon_hash = 0x15d8;
					constexpr auto un_weapon_hash = 0x15dc;
					constexpr auto in_buy_zone = 0x15e0;
					constexpr auto previously_in_buy_zone = 0x15e1;
					constexpr auto aim_punch_angle = 0x1714;
					constexpr auto aim_punch_angle_vel = 0x1720;
					constexpr auto aim_punch_tick_base = 0x172c;
					constexpr auto aim_punch_tick_fraction = 0x1730;
					constexpr auto aim_punch_cache = 0x1738;
					constexpr auto in_landing = 0x15e2;
					constexpr auto landing_time = 0x175c;
					constexpr auto in_hostage_rescue_zone = 0x15e8;
					constexpr auto in_bomb_zone = 0x15e9;
					constexpr auto is_buy_menu_open = 0x15ea;
					constexpr auto time_of_last_injury = 0x15ec;
					constexpr auto next_spray_decal_time = 0x15f0;
					constexpr auto retakes_offering = 0x1758;
					constexpr auto retakes_offering_card = 0x175c;
					constexpr auto retakes_has_defuse_kit = 0x1760;
					constexpr auto retakes_mvplast_round = 0x1761;
					constexpr auto retakes_mvpboost_item = 0x1764;
					constexpr auto retakes_mvpboost_extra_utility = 0x1768;
					constexpr auto need_to_re_apply_gloves = 0x176d;
					constexpr auto econ_gloves = 0x1770;
					constexpr auto must_sync_ragdoll_state = 0x1d21;
					constexpr auto ragdoll_damage_bone = 0x1d24;
					constexpr auto v_ragdoll_damage_force = 0x1d28;
					constexpr auto v_ragdoll_damage_position = 0x1d14;
					constexpr auto ragdoll_damage_weapon_name_arr = 0x1d34;
					constexpr auto ragdoll_damage_headshot = 0x1d74;
					constexpr auto v_ragdoll_server_origin = 0x1d78;
					constexpr auto last_head_bone_transform_is_valid = 0x2368;
					constexpr auto last_land_time = 0x1d84;
					constexpr auto on_ground_last_tick = 0x1d88;
					constexpr auto q_death_eye_angles = 0x1dac;
					constexpr auto skip_one_head_constraint_update = 0x2398;
				}

} // namespace client
} // namespace schema
} // namespace offsets
} // namespace sdk
