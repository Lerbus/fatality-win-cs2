#include <gui/controls/checkbox.h>

GUI_NAMESPACE;

// pride: hover previews the tick in a soft red, a checked box settles on cyan
static color checkbox_on_col()
{
	return colors.theme_id == 1 ? color(84, 185, 219) : colors.accent; // #54b9db
}

static color checkbox_hover_col()
{
	return colors.theme_id == 1 ? color(232, 104, 120) : colors.outline_light; // #e86878
}

void gui::checkbox::render()
{
	control::render();
	if (!is_visible)
		return;

	const auto area_actual = area_abs().floor();
	an->animate();

	auto &d = draw.layers[ctx->content_layer];
	d->add_rect_filled_rounded(area_actual, colors.bg_input, 3.f);

	if (an->value.value.a)
	{
		d->g.anti_alias = true;

		if (!is_radio)
		{
			// tick centred in the 14x14 box
			d->add_line(area_actual.tl() + vec2{3.5f, 7.f}, area_actual.tl() + vec2{6.f, 9.5f}, an->value, 2.f);
			d->add_line(area_actual.tl() + vec2{5.5f, 9.5f}, area_actual.tl() + vec2{10.5f, 4.5f}, an->value, 2.f);
		}
		else
			d->add_rect_filled(area_actual.shrink(4.f), an->value);

		d->g.anti_alias = {};
	}
}

void checkbox::reset()
{
	reset_internal();
	if (callback)
		callback(value);

	run_universal_callbacks();
	hotkeys = value.get_hotkey_table();

	// clean up bad hotkeys
	clean_up_hotkeys();

	// force refresh global elements
	control::update_hotkey_table();
	notify_nearby_label();

	if (ctx->should_update_old_value)
		old_value = *value;
}

void checkbox::on_mouse_enter()
{
	using namespace ren;
	if (value)
		return;

	an->direct(checkbox_hover_col());
}

void checkbox::on_mouse_leave()
{
	using namespace ren;
	if (value)
		return;

	an->direct(checkbox_hover_col().mod_a(0));
}

void checkbox::on_mouse_down(char key)
{
	if (key == m_left && !is_input_locked())
	{
		ctx->do_tick_sound();

		value = !value;
		in_hotkey = false;
		if (callback)
			callback(value);

		run_universal_callbacks();
		reset_internal();
		lock_input();
	}
}

void checkbox::on_mouse_up(char key)
{
	if (key == m_left)
		unlock_input();
}

void checkbox::update_hotkey_table()
{
	clean_up_hotkeys();
	control::update_hotkey_table();
	value.update_hotkeys(hotkeys);
	notify_nearby_label();
}

void checkbox::reset_internal()
{
	an->direct(value ? checkbox_on_col() : checkbox_hover_col().mod_a(is_mouse_on_me ? 255 : 0));
}

void checkbox::set_value(bool v)
{
	value.set(v);
	reset_internal();
}

void checkbox::preserve_hotkey_state()
{
	pres_state.is_engaged = in_hotkey;
	pres_state.hk_value = in_hotkey ? *value : 0;
	pres_state.real_value = in_hotkey ? old_value : *value;

	*value = old_value;
}

void checkbox::restore_hotkey_state()
{
	in_hotkey = pres_state.is_engaged;
	*value = in_hotkey ? pres_state.hk_value : pres_state.real_value;
}

void checkbox::update_hotkey_value(hotkey_update upd, hotkey_info v)
{
	if (upd == hku_click)
	{
		if (v.beh == hkb_toggle)
		{
			// fix bugs
			if (*value && !in_hotkey)
				in_hotkey = true;

			if (in_hotkey)
			{
				old_value = false;
				*value = old_value;
			}
			else
			{
				old_value = true;
				*value = old_value;
			}

			in_hotkey = !in_hotkey;
		}
		else
		{
			if (!in_hotkey)
			{
				old_value = !(bool)v.value;
				*value = (bool)v.value;
			}

			in_hotkey = true;
		}
	}
	else
	{
		if (v.beh == hkb_hold && in_hotkey)
		{
			*value = !(bool)v.value;
			in_hotkey = false;
		}
	}

	reset_internal();
	if (callback)
		callback(value);

	run_universal_callbacks();
}
