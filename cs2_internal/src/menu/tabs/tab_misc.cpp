#include <menu/menu.h>

#include "game/cfg.h"
#include "menu/macros.h"
#include "menu/tab_helpers.h"

using namespace evo::gui;
using namespace evo::gui::helpers;
using namespace evo::ren;

namespace menu
{
	void on_dpi_change(bits &)
	{
		men.update_dpi_scale();
	}
}

namespace
{
	struct misc_ui_state_t
	{
		value_param<bool> enable, prediction{true}, proximity_warning, automatic_release, straight_throw, autoaccept_matches, freecam, untrusted, show_impacts;
		value_param<bits> primary{0}, secondary{0}, armor{0}, utility{0}, equipment{0};
		value_param<color> prediction_color_1{color{80, 30, 255}}, prediction_color_2{color::white()}, proximity_color{color::white()};
		value_param<bool> bunny_hop{true}, jumpbug, edge_jump, peek_assist, standalone_quick_stop;
		value_param<bits> autostrafer{2};
		value_param<float> slowwalk{100.f}, aspect_ratio{1.8f};
		value_param<color> peek_color_1{color{235, 70, 80}}, peek_color_2{color{70, 130, 235}};
		value_param<color> impact_color_1{color{30, 40, 255}}, impact_color_2{color{255, 0, 0}};
		value_param<bits> removals{0}, performance_options{0}, hitsound{2}, hitmarker{0};
		value_param<bool> radar, preserve_killfeed, bomb_timer, spectator_list{true}, team_damage;
		value_param<bits> logs{0}, scoreboard_items{0}, indicators_screen{0}, indicators_crosshair{0};
		value_param<color> screen_color{color{239, 9, 95}}, crosshair_color{color{239, 9, 95}}, bomb_color{color{239, 9, 95}};
	};

	misc_ui_state_t state;

	void misc_autobuy(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("misc>autobuy>enable"), XOR("Enable"), {MAKE("misc>autobuy>enable", checkbox, state.enable)});
		menu::ui::add_combo(e, XOR("misc>autobuy>primary"), XOR("Primary"), state.primary,
			{{XOR("None"), 0}, {XOR("AK47 / M4A1"), 1}, {XOR("SG-556"), 2}, {XOR("AWP"), 3}, {XOR("SSG-08"), 4}});
		menu::ui::add_combo(e, XOR("misc>autobuy>secondary"), XOR("Secondary"), state.secondary,
			{{XOR("None"), 0}, {XOR("Deagle / Revolver"), 1}, {XOR("Dual Berettas"), 2}, {XOR("Tec-9 / Five-SeveN"), 3}, {XOR("P250"), 4}});
		menu::ui::add_combo(e, XOR("misc>autobuy>armor"), XOR("Armor"), state.armor, {{XOR("None"), 0}, {XOR("Kevlar"), 1}, {XOR("Kevlar + helmet"), 2}});
		menu::ui::add_combo(e, XOR("misc>autobuy>utility"), XOR("Utility"), state.utility, {{XOR("None"), 0}, {XOR("Smoke"), 1}, {XOR("Flash"), 2}});
		menu::ui::add_combo(e, XOR("misc>autobuy>equipment"), XOR("Equipment"), state.equipment, {{XOR("None"), 0}, {XOR("Defuser"), 1}, {XOR("Taser"), 2}});
	}

	void misc_grenades(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("misc>grenades>prediction"), XOR("Prediction"),
			{MAKE("misc>grenades>prediction>color-1", color_picker, state.prediction_color_1),
			 MAKE("misc>grenades>prediction>color-2", color_picker, state.prediction_color_2, false),
			 MAKE("misc>grenades>prediction", checkbox, state.prediction)});
		menu::ui::row(
			e, XOR("misc>grenades>proximity-warning"), XOR("Proximity warning"),
			{MAKE("misc>grenades>proximity-color", color_picker, state.proximity_color, false), MAKE("misc>grenades>proximity", checkbox, state.proximity_warning)});
		menu::ui::disabled_row(e, XOR("misc>grenades>automatic-release"), XOR("Automatic release"), {MAKE("misc>grenades>automatic-release", checkbox, state.automatic_release)});
		menu::ui::row(e, XOR("misc>grenades>straight-throw"), XOR("Straight throw"), {MAKE("misc>grenades>straight-throw", checkbox, state.straight_throw)});
	}

	void misc_movement(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("misc>movement>bhop"), XOR("Bunny hop"), {MAKE("misc>movement>bhop", checkbox, state.bunny_hop)});
		menu::ui::row(e, XOR("misc>movement>jumpbug"), XOR("Jumpbug"), {MAKE("misc>movement>jumpbug", checkbox, state.jumpbug)});
		menu::ui::row(e, XOR("misc>movement>edge-jump"), XOR("Edge jump"), {MAKE("misc>movement>edge-jump", checkbox, state.edge_jump)});
		menu::ui::row(
			e, XOR("misc>movement>autostrafer"), XOR("Autostrafer"),
			{MAKE("misc>movement>autostrafer>settings", settings, nullptr),
			 menu::ui::combo(XOR("misc>movement>autostrafer"), state.autostrafer,
				{{XOR("Viewangles"), 0}, {XOR("Movement keys"), 1}, {XOR("Easy strafe"), 2}})});
		menu::ui::row(
			e, XOR("misc>movement>slowwalk"), XOR("Slowwalk"),
			{MAKE("misc>movement>slowwalk", slider<float>, 0.f, 200.f, state.slowwalk, XOR("%.0f%%"), 1.f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("misc>movement>peek-assist"), XOR("Peek assist"),
			{MAKE("misc>movement>peek-assist>settings", settings, nullptr),
			 MAKE("misc>movement>peek-assist>color-1", color_picker, state.peek_color_1, false), MAKE("misc>movement>peek-assist>color-2", color_picker, state.peek_color_2, false),
			 MAKE("misc>movement>peek-assist", checkbox, state.peek_assist)});
		menu::ui::row(e, XOR("misc>movement>standalone-quick-stop"), XOR("Standalone quick stop"), {MAKE("misc>movement>standalone-quick-stop", checkbox, state.standalone_quick_stop)});
	}

	void misc_extra(const std::shared_ptr<layout> &e)
	{
		menu::ui::disabled_row(e, XOR("misc>extra>autoaccept"), XOR("Autoaccept matches"), {MAKE("misc>extra>autoaccept", checkbox, state.autoaccept_matches)});
		menu::ui::disabled_row(e, XOR("misc>extra>freecam"), XOR("Freecam"), {MAKE("misc>extra>freecam", checkbox, state.freecam)});
		const auto untrusted = MAKE("misc>extra>untrusted", checkbox, state.untrusted);
		untrusted->callback = [](bool v) { menu::ui::set_untrusted_features(v); };
		menu::ui::warning_row(e, XOR("misc>extra>untrusted"), XOR("Untrusted features"), {untrusted});
		menu::ui::row(
			e, XOR("misc>extra>aspect-ratio"), XOR("Aspect ratio"),
			{MAKE("misc>extra>aspect-ratio", slider<float>, 1.f, 3.f, state.aspect_ratio, XOR("%.2f"), 0.05f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("misc>extra>impacts"), XOR("Show impacts"),
			{MAKE("misc>extra>impacts>color-1", color_picker, state.impact_color_1, false), MAKE("misc>extra>impacts>color-2", color_picker, state.impact_color_2, false),
			 MAKE("misc>extra>impacts", checkbox, state.show_impacts)});
		menu::ui::add_combo(e, XOR("misc>extra>removals"), XOR("Removals"), state.removals,
			{{XOR("None"), 0}, {XOR("Viewmodel"), 1}, {XOR("Flash"), 2}, {XOR("Smoke"), 3}, {XOR("Blur"), 4}, {XOR("Scope"), 5}}, true);
		menu::ui::add_combo(e, XOR("misc>extra>performance"), XOR("Performance options"), state.performance_options,
			{{XOR("None"), 0}, {XOR("Targets"), 1}, {XOR("Scan accuracy"), 2}}, true);
	}

	void misc_hit_effects(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("misc>hit-effects>hitsound"), XOR("Hitsound"),
			{MAKE("misc>hit-effects>hitsound>settings", settings, nullptr), menu::ui::combo(XOR("misc>hit-effects>hitsound"), state.hitsound,
				{{XOR("None"), 0}, {XOR("Bell"), 1}, {XOR("COD"), 2}, {XOR("Fatality"), 3}, {XOR("Arena switch"), 4}, {XOR("Bepis"), 5},
				 {XOR("Phonk"), 6}, {XOR("Sparkle"), 7}, {XOR("Cock"), 8}})});
		menu::ui::add_combo(e, XOR("misc>hit-effects>hitmarker"), XOR("Hitmarker"), state.hitmarker,
			{{XOR("None"), 0}, {XOR("World"), 1}, {XOR("Crosshair"), 2}});
	}

	void misc_menu(const std::shared_ptr<layout> &e)
	{
		const auto theme = menu::ui::combo(
			XOR("misc>menu>theme"), cfg.misc.theme,
			{{XOR("Default"), cfg_t::theme_default}, {XOR("Pride"), cfg_t::theme_pride},
			 {XOR("Dark"), cfg_t::theme_dark}, {XOR("Hearts"), cfg_t::theme_hearts}});
		theme->legacy_mode = false;
		theme->allow_multiple = false;
		theme->callback = [](bits &b)
		{
			colors.set_theme(b.first_set_bit().value_or((char)cfg_t::theme_default));
		};
		menu::ui::row(e, XOR("misc>menu>theme"), XOR("Theme"), {theme});

		const auto dpi = menu::ui::combo(
			XOR("misc>menu>dpi-scale"), cfg.misc.dpi_scale,
			{{XOR("Automatic"), cfg_t::dpi_automatic}, {XOR("50%"), cfg_t::dpi_50}, {XOR("75%"), cfg_t::dpi_75},
			 {XOR("100%"), cfg_t::dpi_100}, {XOR("125%"), cfg_t::dpi_125}, {XOR("150%"), cfg_t::dpi_150}, {XOR("200%"), cfg_t::dpi_200}});
		dpi->legacy_mode = false;
		dpi->allow_multiple = false;
		dpi->callback = menu::on_dpi_change;
		menu::ui::row(e, XOR("misc>menu>dpi-scale"), XOR("DPI scale"), {dpi});
	}

	void misc_hud(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("misc>hud>radar"), XOR("Radar"), {MAKE("misc>hud>radar", checkbox, state.radar)});
		menu::ui::row(e, XOR("misc>hud>killfeed"), XOR("Preserve killfeed"), {MAKE("misc>hud>killfeed", checkbox, state.preserve_killfeed)});
		menu::ui::add_combo(e, XOR("misc>hud>logs"), XOR("Logs"), state.logs,
			{{XOR("None"), 0}, {XOR("Hurt log"), 1}, {XOR("Buy log"), 2}, {XOR("Debug info"), 3}}, true);
		menu::ui::disabled_row(
			e, XOR("misc>hud>scoreboard"), XOR("Scoreboard items"),
			{menu::ui::combo(XOR("misc>hud>scoreboard"), state.scoreboard_items, {{XOR("None"), 0}, {XOR("Money"), 1}, {XOR("Armor"), 2}, {XOR("Defuser"), 3}})});
	}

	void misc_indicators(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("misc>indicators>screen"), XOR("Screen"),
			{MAKE("misc>indicators>screen>color", color_picker, state.screen_color, false),
			 menu::ui::combo(XOR("misc>indicators>screen"), state.indicators_screen,
				{{XOR("None"), 0}, {XOR("LC"), 1}, {XOR("LBY"), 2}, {XOR("DT"), 3}, {XOR("Hide shot"), 4}, {XOR("Safe point"), 5}, {XOR("Ping"), 6},
				 {XOR("Thirdperson"), 7}, {XOR("Freestand"), 8}, {XOR("Headshot"), 9}, {XOR("Body aim"), 10}, {XOR("Manual"), 11}}, true)});
		menu::ui::row(
			e, XOR("misc>indicators>crosshair"), XOR("Crosshair"),
			{MAKE("misc>indicators>crosshair>color", color_picker, state.crosshair_color, false),
			 menu::ui::combo(XOR("misc>indicators>crosshair"), state.indicators_crosshair,
				{{XOR("None"), 0}, {XOR("LC"), 1}, {XOR("LBY"), 2}, {XOR("DT"), 3}, {XOR("Hide shot"), 4}, {XOR("Safe point"), 5}, {XOR("Ping"), 6},
				 {XOR("Thirdperson"), 7}, {XOR("Freestand"), 8}, {XOR("Headshot"), 9}, {XOR("Body aim"), 10}, {XOR("Manual"), 11}}, true)});
	}

	void misc_windows(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("misc>windows>bomb-timer"), XOR("Bomb timer"),
			{MAKE("misc>windows>bomb-timer>color", color_picker, state.bomb_color, false), MAKE("misc>windows>bomb-timer", checkbox, state.bomb_timer)});
		menu::ui::row(e, XOR("misc>windows>spectator-list"), XOR("Spectator list"), {MAKE("misc>windows>spectator-list", checkbox, state.spectator_list)});
		menu::ui::disabled_row(e, XOR("misc>windows>team-damage"), XOR("Team damage"), {MAKE("misc>windows>team-damage", checkbox, state.team_damage)});
	}
} // namespace

namespace menu::group
{
	void misc_tab(std::shared_ptr<layout> &e)
	{
		const auto area = MAKE("misc>tab", layout, vec2{0.f, 0.f}, vec2{820.f, 482.f}, s_none);
		area->make_not_clip();

		area->add(make_groupbox(XOR("misc>autobuy"), XOR("AUTOBUY"), vec2{0.f, 0.f}, vec2{264.f, 214.f}, misc_autobuy));
		area->add(make_groupbox(XOR("misc>grenades"), XOR("GRENADES"), vec2{0.f, 224.f}, vec2{264.f, 164.f}, misc_grenades));
		area->add(make_groupbox(XOR("misc>hit-effects"), XOR("HIT EFFECTS"), vec2{0.f, 398.f}, vec2{264.f, 84.f}, misc_hit_effects));

		area->add(make_groupbox(XOR("misc>movement"), XOR("MOVEMENT"), vec2{278.f, 0.f}, vec2{264.f, 244.f}, misc_movement));
		area->add(make_groupbox(XOR("misc>extra"), XOR("EXTRA"), vec2{278.f, 255.f}, vec2{264.f, 227.f}, misc_extra));

		area->add(make_groupbox(XOR("misc>menu"), XOR("MENU"), vec2{556.f, 0.f}, vec2{264.f, 90.f}, misc_menu));
		area->add(make_groupbox(XOR("misc>hud"), XOR("HUD"), vec2{556.f, 100.f}, vec2{264.f, 164.f}, misc_hud));
		area->add(make_groupbox(XOR("misc>indicators"), XOR("INDICATORS"), vec2{556.f, 274.f}, vec2{264.f, 90.f}, misc_indicators));
		area->add(make_groupbox(XOR("misc>windows"), XOR("WINDOWS"), vec2{556.f, 374.f}, vec2{264.f, 108.f}, misc_windows));

		e->add(area);
	}
} // namespace menu::group
