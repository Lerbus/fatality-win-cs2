#include <menu/menu.h>
#include <menu/macros.h>
#include <menu/tab_helpers.h>

using namespace evo::gui;
using namespace evo::gui::helpers;
using namespace evo::ren;

namespace
{
	struct local_ui_state_t
	{
		value_param<bool> thirdperson, fov_override, viewmodel_override, penetration_crosshair, force_crosshair, bullet_tracer;
		value_param<bits> overhead_override{0}, remove_scope{0}, spread{0};
		value_param<color> penetration_red{color{255, 0, 0}}, penetration_green{color{0, 255, 0}}, bullet_tracer_color{color{255, 255, 255}};
		value_param<bits> model_visible{0}, model_invisible{0}, model_arms{0}, model_viewmodel{0}, model_attachments{0};
		value_param<color> model_glow_color{color{255, 255, 255}};
	};

	struct player_ui_state_t
	{
		value_param<bool> enabled, dormant, name, box, health, item_name, item_icon, ammo, armor, skeleton, offscreen_arrow, visualize_aimbot, bullet_tracer;
		value_param<bits> info{0}, animations{0};
		value_param<color> name_color{color{255, 255, 255}}, box_color{color{255, 255, 255}}, health_color{color{0, 255, 0}},
			item_name_color{color{255, 255, 255}}, item_icon_color{color{255, 255, 255}}, ammo_color{color{255, 255, 255}},
			armor_color{color{120, 170, 255}}, skeleton_color{color{255, 255, 255}}, offscreen_color{color{255, 255, 255}},
			bullet_tracer_color{color{255, 255, 255}}, model_glow_color{color{70, 145, 255}}, ragdolls_color{color{255, 255, 255}};
		value_param<bits> model_visible, model_hidden{0}, attachments_visible{0}, attachments_hidden{0};
		value_param<bits> backtrack_visible{0}, backtrack_hidden{0}, shot_visible{0}, shot_hidden{0};
		value_param<bool> ragdolls, glow;

		explicit player_ui_state_t(bool enemy = false) :
			model_visible{enemy ? bits{1} : bits{0}} { }
	};

	struct world_ui_state_t
	{
		value_param<bits> esp{0}, glow{0}, weapon_visible{0}, weapon_invisible{0};
		value_param<color> esp_color{color{255, 255, 255}}, glow_color{color{255, 255, 255}};
		value_param<bool> weapon_icons, environment_enabled{true}, fog;
		value_param<float> exposure{39.f};
		value_param<color> world_color_1{color{90, 90, 90}}, world_color_2{color{80, 90, 110}};
		value_param<color> sky_color_1{color{120, 170, 255}}, sky_color_2{color{80, 120, 200}}, sky_color_3{color{255, 255, 255}};
		value_param<bits> world_effects{1}, skybox{0}, disablers{0};
	};

	local_ui_state_t local_ui;
	player_ui_state_t enemy_ui{true}, team_ui{false};
	world_ui_state_t world_ui;

	const std::initializer_list<std::pair<std::string, int>> materials = {
		{XOR("Disabled"), 0}, {XOR("Normal"), 1}, {XOR("Flat"), 2}, {XOR("Rim glow"), 3}, {XOR("Contour"), 4},
		{XOR("Flow"), 5}, {XOR("Acrylic"), 6}, {XOR("Legacy"), 7}, {XOR("Sprite"), 8}};

	void visuals_local_misc(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("visuals>local>misc>thirdperson"), XOR("Thirdperson"),
			{MAKE("visuals>local>thirdperson>settings", settings, nullptr), MAKE("visuals>local>thirdperson", checkbox, local_ui.thirdperson)});
		menu::ui::add_combo(
			e, XOR("visuals>local>overhead-override"), XOR("Overhead override"), local_ui.overhead_override,
			{{XOR("None"), 0}, {XOR("Name"), 1}, {XOR("Weapon"), 2}, {XOR("Name + weapon"), 3}});
		menu::ui::row(
			e, XOR("visuals>local>fov-override"), XOR("Fov override"),
			{MAKE("visuals>local>fov-override>settings", settings, nullptr), MAKE("visuals>local>fov-override", checkbox, local_ui.fov_override)});
		menu::ui::row(
			e, XOR("visuals>local>viewmodel-override"), XOR("Viewmodel override"),
			{MAKE("visuals>local>viewmodel-override>settings", settings, nullptr), MAKE("visuals>local>viewmodel-override", checkbox, local_ui.viewmodel_override)});
		menu::ui::add_combo(
			e, XOR("visuals>local>remove-scope"), XOR("Remove scope"), local_ui.remove_scope,
			{{XOR("None"), 0}, {XOR("Static"), 1}, {XOR("Dynamic"), 2}, {XOR("Gradient static"), 3}, {XOR("Gradient dynamic"), 4}});
		menu::ui::row(
			e, XOR("visuals>local>penetration-crosshair"), XOR("Penetration crosshair"),
			{MAKE("visuals>local>penetration-crosshair>settings", settings, nullptr),
			 MAKE("visuals>local>penetration-crosshair>color-red", color_picker, local_ui.penetration_red, false),
			 MAKE("visuals>local>penetration-crosshair>color-green", color_picker, local_ui.penetration_green, false),
			 MAKE("visuals>local>penetration-crosshair", checkbox, local_ui.penetration_crosshair)});
		menu::ui::row(
			e, XOR("visuals>local>force-crosshair"), XOR("Force crosshair"),
			{MAKE("visuals>local>force-crosshair>settings", settings, nullptr), MAKE("visuals>local>force-crosshair", checkbox, local_ui.force_crosshair)});
		menu::ui::add_combo(
			e, XOR("visuals>local>spread"), XOR("Spread"), local_ui.spread,
			{{XOR("None"), 0}, {XOR("Gradient"), 1}, {XOR("Rainbow"), 2}, {XOR("Rainbow spinning"), 3}});
		menu::ui::row(
			e, XOR("visuals>local>bullet-tracer"), XOR("Bullet tracer"),
			{MAKE("visuals>local>bullet-tracer>color", color_picker, local_ui.bullet_tracer_color, false), MAKE("visuals>local>bullet-tracer", checkbox, local_ui.bullet_tracer)});
	}

	void visuals_local_model(const std::shared_ptr<layout> &e)
	{
		menu::ui::add_combo(e, XOR("visuals>local>model>visible"), XOR("Visible"), local_ui.model_visible, materials);
		menu::ui::add_combo(e, XOR("visuals>local>model>invisible"), XOR("Invisible"), local_ui.model_invisible, materials);
		menu::ui::add_combo(e, XOR("visuals>local>model>arms"), XOR("Arms"), local_ui.model_arms, materials);
		menu::ui::add_combo(e, XOR("visuals>local>model>viewmodel"), XOR("Viewmodel"), local_ui.model_viewmodel, materials);
		menu::ui::add_combo(e, XOR("visuals>local>model>attachments"), XOR("Attachments"), local_ui.model_attachments, materials);
		menu::ui::row(
			e, XOR("visuals>local>model>glow-color"), XOR("Glow color"),
			{MAKE("visuals>local>model>glow-color", color_picker, local_ui.model_glow_color)});
	}

	void player_general(const std::shared_ptr<layout> &e, const std::string &base, player_ui_state_t &ui)
	{
		menu::ui::row(e, base + XOR(">enabled"), XOR("Enabled"), {MAKE_RUNTIME(base + XOR(">enabled"), checkbox, ui.enabled)});
		menu::ui::row(e, base + XOR(">dormant"), XOR("Dormant"), {MAKE_RUNTIME(base + XOR(">dormant"), checkbox, ui.dormant)});
	}

	void player_esp(const std::shared_ptr<layout> &e, const std::string &base, player_ui_state_t &ui, bool enemy)
	{
		const auto color_row = [&](const std::string &key, const std::string &text, value_param<bool> &toggle, value_param<color> &col)
		{
			const std::string k = base + XOR(">") + key;
			std::vector<std::shared_ptr<control>> ctrls{
				MAKE_RUNTIME(k + XOR(">settings"), settings, nullptr),
				MAKE_RUNTIME(k + XOR(">color"), color_picker, col, false),
				MAKE_RUNTIME(k + XOR(">toggle"), checkbox, toggle)};
			menu::ui::row(e, k, text, ctrls);
		};

		color_row(XOR("name"), XOR("Name"), ui.name, ui.name_color);
		color_row(XOR("box"), XOR("Box"), ui.box, ui.box_color);
		color_row(XOR("health"), XOR("Health"), ui.health, ui.health_color);
		color_row(XOR("item-name"), XOR("Item name"), ui.item_name, ui.item_name_color);
		color_row(XOR("item-icon"), XOR("Item icon"), ui.item_icon, ui.item_icon_color);
		color_row(XOR("ammo"), XOR("Ammo"), ui.ammo, ui.ammo_color);
		color_row(XOR("armor"), XOR("Armor"), ui.armor, ui.armor_color);
		menu::ui::add_combo(e, base + XOR(">info"), XOR("Info"), ui.info,
			{{XOR("None"), 0}, {XOR("Money"), 1}, {XOR("Armor"), 2}, {XOR("Defuser"), 3}, {XOR("Taser"), 4}}, true);
		color_row(XOR("skeleton"), XOR("Skeleton"), ui.skeleton, ui.skeleton_color);

		if (enemy)
		{
			color_row(XOR("offscreen-arrow"), XOR("Offscreen arrow"), ui.offscreen_arrow, ui.offscreen_color);
			color_row(XOR("visualize-aimbot"), XOR("Visualize aimbot"), ui.visualize_aimbot, ui.bullet_tracer_color);
		}

		color_row(XOR("bullet-tracer"), XOR("Bullet tracer"), ui.bullet_tracer, ui.bullet_tracer_color);
	}

	void player_model(const std::shared_ptr<layout> &e, const std::string &base, player_ui_state_t &ui, bool with_backtrack)
	{
		menu::ui::row(
			e, base + XOR(">model>visible"), XOR("Visible"),
			{MAKE_RUNTIME(base + XOR(">model>visible-glow>settings"), settings, nullptr),
			 MAKE_RUNTIME(base + XOR(">model>visible-glow>color"), color_picker, ui.model_glow_color, false),
			 menu::ui::combo(base + XOR(">model>visible"), ui.model_visible, materials)});
		menu::ui::add_combo(e, base + XOR(">model>hidden"), XOR("Hidden"), ui.model_hidden, materials);

		if (!with_backtrack)
			return;

		menu::ui::add_combo(e, base + XOR(">model>backtrack-visible"), XOR("Backtrack visible"), ui.backtrack_visible, materials);
		menu::ui::add_combo(e, base + XOR(">model>backtrack-hidden"), XOR("Backtrack hidden"), ui.backtrack_hidden, materials);
	}

	void player_attachments(const std::shared_ptr<layout> &e, const std::string &base, player_ui_state_t &ui)
	{
		menu::ui::add_combo(e, base + XOR(">attachments>visible"), XOR("Visible"), ui.attachments_visible, materials);
		menu::ui::add_combo(e, base + XOR(">attachments>hidden"), XOR("Hidden"), ui.attachments_hidden, materials);
	}

	void player_extra(const std::shared_ptr<layout> &e, const std::string &base, player_ui_state_t &ui)
	{
		menu::ui::row(
			e, base + XOR(">extra>ragdolls"), XOR("Ragdolls"),
			{MAKE_RUNTIME(base + XOR(">extra>ragdolls>color"), color_picker, ui.ragdolls_color, false), MAKE_RUNTIME(base + XOR(">extra>ragdolls"), checkbox, ui.ragdolls)});
		menu::ui::row(
			e, base + XOR(">extra>glow"), XOR("Glow"),
			{MAKE_RUNTIME(base + XOR(">extra>glow>settings"), settings, nullptr),
			 MAKE_RUNTIME(base + XOR(">extra>glow>color"), color_picker, ui.model_glow_color, false),
			 MAKE_RUNTIME(base + XOR(">extra>glow"), checkbox, ui.glow)});
	}

	void enemy_model(const std::shared_ptr<layout> &e)
	{
		player_model(e, XOR("visuals>enemy"), enemy_ui, false);
	}

	void enemy_attachments(const std::shared_ptr<layout> &e)
	{
		player_attachments(e, XOR("visuals>enemy"), enemy_ui);
	}

	void enemy_backtrack(const std::shared_ptr<layout> &e)
	{
		menu::ui::add_combo(e, XOR("visuals>enemy>backtrack>visible"), XOR("Visible"), enemy_ui.backtrack_visible, materials);
		menu::ui::add_combo(e, XOR("visuals>enemy>backtrack>hidden"), XOR("Hidden"), enemy_ui.backtrack_hidden, materials);
	}

	void enemy_shot(const std::shared_ptr<layout> &e)
	{
		menu::ui::add_combo(e, XOR("visuals>enemy>shot>visible"), XOR("Visible"), enemy_ui.shot_visible, materials);
		menu::ui::add_combo(e, XOR("visuals>enemy>shot>hidden"), XOR("Hidden"), enemy_ui.shot_hidden, materials);
	}

	void enemy_extra(const std::shared_ptr<layout> &e)
	{
		player_extra(e, XOR("visuals>enemy"), enemy_ui);
	}

	void team_model(const std::shared_ptr<layout> &e)
	{
		player_model(e, XOR("visuals>team"), team_ui, true);
	}

	void team_attachments(const std::shared_ptr<layout> &e)
	{
		player_attachments(e, XOR("visuals>team"), team_ui);
	}

	void team_extra(const std::shared_ptr<layout> &e)
	{
		player_extra(e, XOR("visuals>team"), team_ui);
	}

	void player_disablers(const std::shared_ptr<layout> &e, player_ui_state_t &ui)
	{
		menu::ui::add_combo(
			e, XOR("visuals>player>disablers>animations"), XOR("Animations"), ui.animations,
			{{XOR("None"), 0}, {XOR("Move lean"), 1}, {XOR("Fake"), 2}, {XOR("Fake opposite"), 3}, {XOR("Jitter"), 4}});
	}

	void enemy_general(const std::shared_ptr<layout> &e)
	{
		player_general(e, XOR("visuals>enemy"), enemy_ui);
	}

	void enemy_esp(const std::shared_ptr<layout> &e)
	{
		player_esp(e, XOR("visuals>enemy"), enemy_ui, true);
	}

	void enemy_disablers(const std::shared_ptr<layout> &e)
	{
		player_disablers(e, enemy_ui);
	}

	void team_general(const std::shared_ptr<layout> &e)
	{
		player_general(e, XOR("visuals>team"), team_ui);
	}

	void team_esp(const std::shared_ptr<layout> &e)
	{
		player_esp(e, XOR("visuals>team"), team_ui, false);
	}

	void team_disablers(const std::shared_ptr<layout> &e)
	{
		player_disablers(e, team_ui);
	}

	void world_esp_group(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("visuals>world>esp"), XOR("ESP"),
			{MAKE("visuals>world>esp", color_picker, world_ui.esp_color, false),
			 menu::ui::combo(XOR("visuals>world>esp-type"), world_ui.esp,
				{{XOR("None"), 0}, {XOR("Weapons"), 1}, {XOR("Grenades"), 2}, {XOR("C4"), 3}, {XOR("Danger zone"), 4}})});
		menu::ui::row(e, XOR("visuals>world>weapon-icons"), XOR("Weapon icons"), {MAKE("visuals>world>weapon-icons", checkbox, world_ui.weapon_icons)});
		menu::ui::add_combo(e, XOR("visuals>world>weapon-visible"), XOR("Weapon visible"), world_ui.weapon_visible, materials);
		menu::ui::add_combo(e, XOR("visuals>world>weapon-invisible"), XOR("Weapon invisible"), world_ui.weapon_invisible, materials);
		menu::ui::row(
			e, XOR("visuals>world>glow"), XOR("Glow"),
			{MAKE("visuals>world>glow", color_picker, world_ui.glow_color, false),
			 menu::ui::combo(XOR("visuals>world>glow-type"), world_ui.glow, {{XOR("None"), 0}, {XOR("Outline"), 1}, {XOR("Filled"), 2}})});
	}

	void world_environment(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("visuals>world>environment"), XOR("Enabled"), {MAKE("visuals>world>environment", checkbox, world_ui.environment_enabled)});
		menu::ui::row(
			e, XOR("visuals>world>exposure"), XOR("Exposure"),
			{MAKE("visuals>world>exposure", slider<float>, 0.f, 100.f, world_ui.exposure, XOR("%.0f%%"), 1.f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("visuals>world>world"), XOR("World"),
			{MAKE("visuals>world>world>color-1", color_picker, world_ui.world_color_1, false),
			 MAKE("visuals>world>world>color-2", color_picker, world_ui.world_color_2, false)});
		menu::ui::row(
			e, XOR("visuals>world>fog"), XOR("Fog"),
			{MAKE("visuals>world>fog>settings", settings, nullptr), MAKE("visuals>world>fog", checkbox, world_ui.fog)});
		menu::ui::add_combo(e, XOR("visuals>world>effects"), XOR("World effects"), world_ui.world_effects,
			{{XOR("None"), 0}, {XOR("Snow"), 1}, {XOR("Rain"), 2}, {XOR("Ash"), 3}});
		menu::ui::row(
			e, XOR("visuals>world>sky"), XOR("Sky"),
			{MAKE("visuals>world>sky>color-1", color_picker, world_ui.sky_color_1, false),
			 MAKE("visuals>world>sky>color-2", color_picker, world_ui.sky_color_2, false),
			 MAKE("visuals>world>sky>color-3", color_picker, world_ui.sky_color_3, false)});
		menu::ui::add_combo(e, XOR("visuals>world>skybox"), XOR("Skybox"), world_ui.skybox,
			{{XOR("Default"), 0}, {XOR("Night"), 1}, {XOR("Cloudy"), 2}, {XOR("Custom"), 3}});
		menu::ui::add_combo(e, XOR("visuals>world>disablers"), XOR("Disablers"), world_ui.disablers,
			{{XOR("None"), 0}, {XOR("Model fade"), 1}, {XOR("Dormancy interp"), 2}, {XOR("Health bar interp"), 3}, {XOR("ESP fade"), 4}}, true);
	}

	std::shared_ptr<layout> make_child_area(uint64_t id, bool active)
	{
		const auto area = std::make_shared<layout>(control_id{id}, vec2{114.f, 0.f}, vec2{706.f, 482.f}, s_none);
		area->is_visible = active;
		area->make_not_clip();
		return area;
	}
} // namespace

namespace menu::group
{
	void visuals_tab(std::shared_ptr<layout> &e)
	{
		const auto tabs = MAKE("visuals>tabs", tabs_layout, vec2{0.f, 0.f}, vec2{113.f, 482.f}, td_vertical);
		tabs->add(MAKE("visuals>tabs>local", child_tab, XOR("Local"), GUI_HASH("visuals>tab>local"))->make_vertical()->make_selected());
		tabs->add(MAKE("visuals>tabs>enemy", child_tab, XOR("Enemy"), GUI_HASH("visuals>tab>enemy"))->make_vertical());
		tabs->add(MAKE("visuals>tabs>team", child_tab, XOR("Team"), GUI_HASH("visuals>tab>team"))->make_vertical());
		tabs->add(MAKE("visuals>tabs>world", child_tab, XOR("World"), GUI_HASH("visuals>tab>world"))->make_vertical());
		e->add(tabs);

		const auto local = make_child_area(GUI_HASH("visuals>tab>local"), true);
		local->add(make_groupbox(XOR("visuals>local>misc"), XOR("MISC"), vec2{0.f, 0.f}, vec2{264.f, 482.f}, visuals_local_misc));
		local->add(make_groupbox(XOR("visuals>local>model"), XOR("MODEL"), vec2{278.f, 0.f}, vec2{264.f, 482.f}, visuals_local_model));
		e->add(local);

		const auto enemy = make_child_area(GUI_HASH("visuals>tab>enemy"), false);
		enemy->add(make_groupbox(XOR("visuals>enemy>general"), XOR("GENERAL"), vec2{0.f, 0.f}, vec2{226.f, 90.f}, enemy_general));
		enemy->add(make_groupbox(XOR("visuals>enemy>esp"), XOR("ESP"), vec2{0.f, 100.f}, vec2{226.f, 382.f}, enemy_esp));
		enemy->add(make_groupbox(XOR("visuals>enemy>preview"), XOR("PREVIEW"), vec2{240.f, 0.f}, vec2{226.f, 408.f}, [](const std::shared_ptr<layout> &) { }));
		enemy->add(make_groupbox(XOR("visuals>enemy>disablers"), XOR("DISABLERS"), vec2{240.f, 418.f}, vec2{226.f, 64.f}, enemy_disablers));
		enemy->add(make_groupbox(XOR("visuals>enemy>model"), XOR("MODEL"), vec2{480.f, 0.f}, vec2{226.f, 90.f}, enemy_model));
		enemy->add(make_groupbox(XOR("visuals>enemy>attachments"), XOR("ATTACHMENTS"), vec2{480.f, 100.f}, vec2{226.f, 90.f}, enemy_attachments));
		enemy->add(make_groupbox(XOR("visuals>enemy>backtrack"), XOR("BACKTRACK"), vec2{480.f, 200.f}, vec2{226.f, 90.f}, enemy_backtrack));
		enemy->add(make_groupbox(XOR("visuals>enemy>shot"), XOR("SHOT"), vec2{480.f, 300.f}, vec2{226.f, 90.f}, enemy_shot));
		enemy->add(make_groupbox(XOR("visuals>enemy>extra"), XOR("EXTRA"), vec2{480.f, 400.f}, vec2{226.f, 82.f}, enemy_extra));
		e->add(enemy);

		const auto team = make_child_area(GUI_HASH("visuals>tab>team"), false);
		team->add(make_groupbox(XOR("visuals>team>general"), XOR("GENERAL"), vec2{0.f, 0.f}, vec2{226.f, 90.f}, team_general));
		team->add(make_groupbox(XOR("visuals>team>esp"), XOR("ESP"), vec2{0.f, 100.f}, vec2{226.f, 382.f}, team_esp));
		team->add(make_groupbox(XOR("visuals>team>preview"), XOR("PREVIEW"), vec2{240.f, 0.f}, vec2{226.f, 408.f}, [](const std::shared_ptr<layout> &) { }));
		team->add(make_groupbox(XOR("visuals>team>disablers"), XOR("DISABLERS"), vec2{240.f, 418.f}, vec2{226.f, 64.f}, team_disablers));
		team->add(make_groupbox(XOR("visuals>team>model"), XOR("MODEL"), vec2{480.f, 0.f}, vec2{226.f, 158.f}, team_model));
		team->add(make_groupbox(XOR("visuals>team>attachments"), XOR("ATTACHMENTS"), vec2{480.f, 168.f}, vec2{226.f, 158.f}, team_attachments));
		team->add(make_groupbox(XOR("visuals>team>extra"), XOR("EXTRA"), vec2{480.f, 336.f}, vec2{226.f, 146.f}, team_extra));
		e->add(team);

		const auto world = make_child_area(GUI_HASH("visuals>tab>world"), false);
		world->add(make_groupbox(XOR("visuals>world>esp"), XOR("ESP"), vec2{0.f, 0.f}, vec2{264.f, 200.f}, world_esp_group));
		world->add(make_groupbox(XOR("visuals>world>environment"), XOR("ENVIRONMENT"), vec2{0.f, 220.f}, vec2{264.f, 262.f}, world_environment));
		e->add(world);
	}
} // namespace menu::group
