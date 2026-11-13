#include <menu/menu.h>
#include <menu/macros.h>
#include <menu/tab_helpers.h>

using namespace evo::gui;
using namespace evo::gui::helpers;
using namespace evo::ren;

namespace
{
	struct rage_ui_state_t
	{
		value_param<float> hit_chance{0.f}, pointscale{4.f}, min_damage{0.f}, maximum_fov{0.f}, spin{0.f};
		value_param<bits> hitboxes, multipoint, target_selection{0};
		value_param<bool> autostop, autoscope, ignore_limbs, autorevolver;
		value_param<bool> aimbot, silent_aim, autofire, delay_shot, duck_peek_assist, force_bodyaim, force_shoot, headshot_only, knife_bot,
			zeus_bot, nospread, doubletap;
		value_param<bool> anti_aim, hide_shot{true}, override_left, override_right, override_back, override_forward;
		value_param<bits> pitch{0}, pitch_jitter{0}, yaw_base{0}, yaw{0}, yaw_jitter{0}, jitter_disabler{0};
		value_param<color> mouse_override_color_1{color{255, 0, 0}}, mouse_override_color_2{color{0, 255, 0}};
	};

	rage_ui_state_t ui;

	void rage_aimbot_weapon(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("rage>aimbot>weapon>hit-chance"), XOR("Hit-chance"),
			{MAKE("rage>weapon>hit-chance", slider<float>, 0.f, 100.f, ui.hit_chance,
				ssa<float>{ssv<float>{1.f, XOR("%.0f%%")}, ssv<float>{0.f, XOR("Auto")}}, 1.f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("rage>weapon>pointscale"), XOR("Pointscale"),
			{MAKE("rage>weapon>pointscale", slider<float>, 0.f, 100.f, ui.pointscale, XOR("%.0f%%"), 1.f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("rage>weapon>min-damage"), XOR("Min-damage"),
			{MAKE("rage>weapon>min-damage", slider<float>, 0.f, 125.f, ui.min_damage,
				ssa<float>{ssv<float>{101.f, XOR("HP + %.0f")}, ssv<float>{1.f, XOR("%.0f hp")}, ssv<float>{0.f, XOR("Lethal")}}, 1.f,
				vec2{80.f, 16.f})});
		menu::ui::add_combo(
			e, XOR("rage>weapon>hitboxes"), XOR("Hitboxes"), ui.hitboxes,
			{{XOR("Head"), 0}, {XOR("Chest"), 1}, {XOR("Stomach"), 2}, {XOR("Arms"), 3}, {XOR("Legs"), 4}, {XOR("Feet"), 5}}, true);
		menu::ui::add_combo(
			e, XOR("rage>weapon>multipoint"), XOR("Multipoint"), ui.multipoint,
			{{XOR("Head"), 0}, {XOR("Chest"), 1}, {XOR("Stomach"), 2}, {XOR("Arms"), 3}, {XOR("Legs"), 4}, {XOR("Feet"), 5}}, true);
		menu::ui::add_combo(
			e, XOR("rage>weapon>target-selection"), XOR("Target selection"), ui.target_selection,
			{{XOR("Damage"), 0}, {XOR("Distance"), 1}, {XOR("Health"), 2}});
	}

	void rage_aimbot_extra(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(
			e, XOR("rage>aimbot>extra>autostop"), XOR("Autostop"),
			{MAKE("rage>aimbot>extra>autostop>settings", settings, nullptr), MAKE("rage>aimbot>extra>autostop", checkbox, ui.autostop)});
		menu::ui::row(e, XOR("rage>extra>autoscope"), XOR("Autoscope"), {MAKE("rage>aimbot>extra>autoscope", checkbox, ui.autoscope)});
		menu::ui::row(e, XOR("rage>aimbot>ignore-limbs"), XOR("Ignore limbs on moving"), {MAKE("rage>extra>ignore-limbs", checkbox, ui.ignore_limbs)});
		menu::ui::row(e, XOR("rage>aimbot>extra>autorevolver"), XOR("Autorevolver"), {MAKE("rage>aimbot>extra>autorevolver", checkbox, ui.autorevolver)});
	}

	void rage_general(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("rage>general>aimbot"), XOR("Aimbot"), {MAKE("rage>aimbot>general>aimbot", checkbox, ui.aimbot)});
		const auto silent_aim = MAKE("rage>aimbot>general>silent-aim", checkbox, ui.silent_aim);
		menu::ui::untrusted_row(e, XOR("rage>aimbot>general>silent-aim"), XOR("Silent aim"), {silent_aim}, silent_aim);
		menu::ui::row(
			e, XOR("rage>aimbot>general>fov"), XOR("Maximum fov"),
			{MAKE("rage>aimbot>general>maximum-fov", slider<float>, 0.f, 180.f, ui.maximum_fov, XOR("%.0f deg"), 1.f, vec2{80.f, 16.f})});

		menu::ui::row(e, XOR("rage>general>autofire"), XOR("Autofire"), {MAKE("rage>aimbot>general>autofire", checkbox, ui.autofire)});
		menu::ui::row(e, XOR("rage>general>delay-shot"), XOR("Delay shot"), {MAKE("rage>aimbot>general>delay-shot", checkbox, ui.delay_shot)});
		menu::ui::row(e, XOR("rage>general>duck-peek-assist"), XOR("Duck peek assist"), {MAKE("rage>general>duck-peek-assist", checkbox, ui.duck_peek_assist)});
		menu::ui::row(e, XOR("rage>general>force-bodyaim"), XOR("Force bodyaim"), {MAKE("rage>general>force-bodyaim", checkbox, ui.force_bodyaim)});
		menu::ui::row(e, XOR("rage>general>force-shoot"), XOR("Force shoot"), {MAKE("rage>general>force-shoot", checkbox, ui.force_shoot)});
		menu::ui::row(e, XOR("rage>general>headshot-only"), XOR("Headshot only"), {MAKE("rage>general>headshot-only", checkbox, ui.headshot_only)});
		menu::ui::row(e, XOR("rage>general>knife-bot"), XOR("Knife bot"), {MAKE("rage>general>knife-bot", checkbox, ui.knife_bot)});
		menu::ui::row(e, XOR("rage>general>zeus-bot"), XOR("Zeus bot"), {MAKE("rage>general>zeus-bot", checkbox, ui.zeus_bot)});
		menu::ui::row(
			e, XOR("rage>general>nospread"), XOR("Nospread"),
			{MAKE("rage>general>nospread>settings", settings, nullptr), MAKE("rage>general>nospread", checkbox, ui.nospread)});
		const auto doubletap = MAKE("rage>general>doubletap", checkbox, ui.doubletap);
		menu::ui::untrusted_row(
			e, XOR("rage>general>doubletap"), XOR("Doubletap"),
			{MAKE("rage>general>doubletap>settings", settings, nullptr), doubletap}, doubletap);
	}

	void rage_angles(const std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("rage>anti-aim>angles>anti-aim"), XOR("Anti-aim"), {MAKE("rage>angles>anti-aim", checkbox, ui.anti_aim)});
		menu::ui::add_combo(e, XOR("rage>angles>pitch"), XOR("Pitch"), ui.pitch,
			{{XOR("None"), 0}, {XOR("Down"), 1}, {XOR("Up"), 2}, {XOR("Zero"), 3}, {XOR("Custom"), 4}});
		menu::ui::add_combo(e, XOR("rage>angles>pitch-jitter"), XOR("Pitch jitter"), ui.pitch_jitter,
			{{XOR("None"), 0}, {XOR("Center"), 1}, {XOR("Offset"), 2}});
		menu::ui::add_combo(e, XOR("rage>angles>yaw-base"), XOR("Yaw base"), ui.yaw_base, {{XOR("Viewangles"), 0}, {XOR("At target"), 1}});
		menu::ui::add_combo(e, XOR("rage>angles>yaw"), XOR("Yaw"), ui.yaw, {{XOR("None"), 0}, {XOR("Backwards"), 1}, {XOR("Custom"), 2}});
		menu::ui::add_combo(
			e, XOR("rage>angles>yaw-jitter"), XOR("Yaw jitter"), ui.yaw_jitter,
			{{XOR("None"), 0}, {XOR("Center"), 1}, {XOR("Offset"), 2}});
		menu::ui::add_combo(
			e, XOR("rage>angles>jitter-disabler"), XOR("Jitter disabler"), ui.jitter_disabler,
			{{XOR("Yaw change"), 0}, {XOR("Pitch change"), 1}});
		const auto hide_shot = MAKE("rage>angles>hide-shot", checkbox, ui.hide_shot);
		menu::ui::untrusted_row(e, XOR("rage>angles>hide-shot"), XOR("Hide shot"), {hide_shot}, hide_shot);
		menu::ui::row(
			e, XOR("rage>angles>spin"), XOR("Spin"),
			{MAKE("rage>angles>spin", slider<float>, 0.f, 100.f, ui.spin, XOR("%.0f%%"), 1.f, vec2{80.f, 16.f})});
		menu::ui::row(
			e, XOR("rage>angles>mouse-override"), XOR("Mouse override"),
			{MAKE("rage>angles>mouse-override>settings", settings, nullptr),
			 MAKE("rage>angles>mouse-override>color-1", color_picker, ui.mouse_override_color_1, false),
			 MAKE("rage>angles>mouse-override>color-2", color_picker, ui.mouse_override_color_2, false)});
		menu::ui::row(e, XOR("rage>angles>override-left"), XOR("Override left"), {MAKE("rage>angles>override-left", checkbox, ui.override_left)});
		menu::ui::row(e, XOR("rage>angles>override-right"), XOR("Override right"), {MAKE("rage>angles>override-right", checkbox, ui.override_right)});
		menu::ui::row(e, XOR("rage>angles>override-back"), XOR("Override back"), {MAKE("rage>angles>override-back", checkbox, ui.override_back)});
		menu::ui::row(e, XOR("rage>angles>override-forward"), XOR("Override forward"), {MAKE("rage>angles>override-forward", checkbox, ui.override_forward)});
	}
} // namespace

namespace menu::group
{
	void rebuild_rage_layout(bool) { }

	void rage_tab(std::shared_ptr<layout> &e)
	{
		const auto tabs = MAKE("rage>tabs", tabs_layout, vec2{0.f, 0.f}, vec2{820.f, 24.f}, td_horizontal);
		tabs->add(MAKE("rage>tabs>aimbot", child_tab, XOR("Aimbot"), GUI_HASH("rage>tab>aimbot"))->make_selected());
		tabs->add(MAKE("rage>tabs>anti-aim", child_tab, XOR("Anti-aim"), GUI_HASH("rage>tab>anti-aim")));
		e->add(tabs);

		const auto aimbot = MAKE("rage>tab>aimbot", layout, vec2{0.f, 35.f}, vec2{820.f, 444.f}, s_none);
		aimbot->make_not_clip();

		const auto weapons = MAKE("rage>weapons", tabs_layout, vec2{0.f, 2.f}, vec2{113.f, 407.f}, td_vertical);
		weapons->add(MAKE("rage>weapons>general", child_tab, XOR("General"), GUI_HASH("rage>weapon>general"))->make_vertical()->make_selected());
		aimbot->add(weapons);

		const auto content = MAKE("rage>aimbot>content", layout, vec2{114.f, 0.f}, vec2{706.f, 444.f}, s_none);
		content->make_not_clip();

		const auto weapon_extra = MAKE("rage>weapon>general", layout, vec2{0.f, 0.f}, vec2{466.f, 444.f}, s_none);
		weapon_extra->make_not_clip();
		weapon_extra->add(make_groupbox(XOR("rage>weapon"), XOR("WEAPON"), vec2{0.f, 0.f}, vec2{226.f, 444.f}, rage_aimbot_weapon));
		weapon_extra->add(make_groupbox(XOR("rage>extra"), XOR("EXTRA"), vec2{240.f, 0.f}, vec2{226.f, 444.f}, rage_aimbot_extra));
		content->add(weapon_extra);
		content->add(make_groupbox(XOR("rage>aimbot>general"), XOR("GENERAL"), vec2{480.f, 0.f}, vec2{226.f, 444.f}, rage_general));
		aimbot->add(content);
		// "+ Add" is plain text only, without the control well behind it
		const auto add_override = MAKE("rage>override", button, XOR("+ Add"), nullptr, vec2{0.f, 417.f}, vec2{101.f, 20.f});
		add_override->no_bg = true;
		aimbot->add(add_override);
		e->add(aimbot);

		const auto anti_aim = MAKE("rage>tab>anti-aim", layout, vec2{0.f, 35.f}, vec2{820.f, 444.f}, s_none);
		anti_aim->is_visible = false;
		anti_aim->make_not_clip();
		anti_aim->add(make_groupbox(XOR("rage>anti-aim>angles"), XOR("ANGLES"), vec2{0.f, 0.f}, vec2{264.f, 444.f}, rage_angles));
		e->add(anti_aim);
	}
} // namespace menu::group
