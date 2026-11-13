#include <gui/controls/slider.h>

GUI_NAMESPACE;

#define MAKE_TEMPLATE_FN_DEF(r, f) template r slider<int>::f ; template r slider<float>::f ; template r slider<double>::f ;

template <typename T>
void slider<T>::on_mouse_down(char key)
{
	if (key == m_left)
	{
		lock_input();
		update_value();
	}
}

template <typename T>
void slider<T>::on_mouse_move(const vec2 &p, const vec2 &d)
{
	if (is_locked_by_me())
		update_value();
}

template <typename T>
void slider<T>::preserve_hotkey_state()
{
	pres_state.is_engaged = in_hotkey;
	*(float *)&pres_state.hk_value = in_hotkey ? *value : 0;
	*(float *)&pres_state.real_value = in_hotkey ? old_value : *value;

	*value = old_value;
}

template <typename T>
void slider<T>::restore_hotkey_state()
{
	in_hotkey = pres_state.is_engaged;
	*value = in_hotkey ? *(T *)&pres_state.hk_value : *(T *)&pres_state.real_value;
}

template <typename T>
void slider<T>::on_key_down(uint16_t key)
{
	if (!is_mouse_on_me)
		return;

	const auto o = *value;
	if (key == VK_LEFT || key == VK_DOWN)
		*value -= step;
	if (key == VK_RIGHT || key == VK_UP)
		*value += step;

	*value = std::clamp(*value, low, high);
	if (o != *value)
	{
		if (callback)
			callback(o, *value);

		in_hotkey = false;
		run_universal_callbacks();
		ctx->do_tick_sound();
	}
}

template <typename T>
void slider<T>::on_mouse_wheel(float factor)
{
	// don't do anything if control is down.
	if (!input.is_key_down(VK_CONTROL))
		return;

	const auto o = *value;
	*value += step * factor;
	*value = std::clamp(*value, low, high);

	if (o != *value)
	{
		if (callback)
			callback(o, *value);

		in_hotkey = false;
		run_universal_callbacks();
		ctx->do_tick_sound();
	}
}

template <typename T>
void slider<T>::on_mouse_up(char key)
{
	if (key == m_left)
	{
		unlock_input();
		ctx->do_tick_sound();
	}
}

template <typename T>
void slider<T>::render()
{
	control::render();

	const auto r = area_abs().floor();
	const auto f = get_fill();

	auto &d = draw.layers[ctx->content_layer];
	d->add_rect_filled_rounded(r, colors.bg_input, 3.f);

	d->g.anti_alias = true;
	const auto fill_r = r.size(f).shrink(2.f).floor();

	// pride: every slider samples the same pink -> blue ramp, mapped over the
	// full slider width so the fill reveals the ramp up to the current value
	const auto &grad = ctx->res.general.slider_gradient;
	const bool use_grad = colors.theme_id == 1 && grad && grad->obj && r.width() > 0.f;
	const auto fill_col = use_grad ? color::white() : colors.slider_fill(id);
	if (use_grad)
	{
		d->g.texture = grad->obj;
		d->g.uv_rect = rect{(fill_r.mins.x - r.mins.x) / r.width(), 0.f, (fill_r.maxs.x - r.mins.x) / r.width(), 1.f};
	}

	// draw at any width; shrink the rounding so tiny fills stay visible
	if (fill_r.width() > 0.f)
		d->add_rect_filled_rounded(fill_r, fill_col, std::min(2.f, fill_r.width() * 0.5f));

	d->g.texture = {};
	d->g.uv_rect = {};
	d->g.anti_alias = false;

	const auto m = draw.fonts[GUI_HASH("gui_main")];
	const auto value_s = get_formatted(*value);

	const auto old_clip = d->g.clip_rect;

	d->font = m;
	d->add_text(r.center(), value_s, colors.text_dark, text_params::with_vh(align_center, align_center));
	// clip the bright half of the value text to the accent fill itself, so the
	// text highlight can never spill past the coloured bar (or stop short of it)
	// skip when there is no fill: an empty/inverted clip rect would let the
	// bright copy draw over the whole box
	if (fill_r.width() > 0.f)
	{
		d->override_clip_rect(fill_r);
		d->add_text(r.center(), value_s, colors.text, text_params::with_vh(align_center, align_center));
		d->g.clip_rect = old_clip;
	}
}

template <typename T>
void slider<T>::update_value()
{
	const auto step_frac = 1.f / step;
	const auto cursor = input.cursor();

	const auto o = *value;

	*value = (cursor.x - pos_abs().x) / size.x * ((float)high - (float)low) + (float)low;
	*value = std::clamp(floorf(*value * step_frac + 0.5f) / step_frac, (float)low, (float)high);

	if (o != *value)
	{
		if (callback)
			callback(o, *value);

		in_hotkey = false;
		run_universal_callbacks();
	}
}

template <typename T>
void slider<T>::determine_type()
{
	hk_type = std::is_floating_point<T>() ? st_float : st_int;
}

template <typename T>
void slider<T>::reset()
{
	if (callback)
		callback(value, value);

	hotkeys = value.get_hotkey_table();
	clean_up_hotkeys();
	control::update_hotkey_table();
	run_universal_callbacks();
	notify_nearby_label();

	if (!handling_hotkey && ctx->should_update_old_value)
	{
		old_value = *value;
		in_hotkey = false;
	}
}

template <typename T>
void slider<T>::update_hotkey_table()
{
	clean_up_hotkeys();
	control::update_hotkey_table();
	value.update_hotkeys(hotkeys);
	notify_nearby_label();
}

template <typename T>
void slider<T>::update_hotkey_value(hotkey_update upd, hotkey_info v)
{
	if (upd == hku_click)
	{
		if (v.beh == hkb_toggle)
		{
			if (in_hotkey)
				*value = old_value;
			else
			{
				old_value = *value;
				*value = *(T *)&v.value;
			}

			in_hotkey = !in_hotkey;
		}
		else
		{
			if (!in_hotkey)
			{
				old_value = *value;
				*value = *(T *)&v.value;
			}

			in_hotkey = true;
		}
	}
	else
	{
		if (v.beh == hkb_hold && in_hotkey)
		{
			*value = old_value;
			in_hotkey = false;
		}
	}

	handling_hotkey = true;
	reset();
	handling_hotkey = false;
}

MAKE_TEMPLATE_FN_DEF(void, on_mouse_down(char key));
MAKE_TEMPLATE_FN_DEF(void, on_mouse_move(const ren::vec2 &p, const ren::vec2 &d));
MAKE_TEMPLATE_FN_DEF(void, on_key_down(uint16_t key));
MAKE_TEMPLATE_FN_DEF(void, on_mouse_up(char key));
MAKE_TEMPLATE_FN_DEF(void, on_mouse_wheel(float factor));
MAKE_TEMPLATE_FN_DEF(void, render());
MAKE_TEMPLATE_FN_DEF(void, update_value());
MAKE_TEMPLATE_FN_DEF(void, determine_type());
MAKE_TEMPLATE_FN_DEF(void, reset());
MAKE_TEMPLATE_FN_DEF(void, update_hotkey_table());
MAKE_TEMPLATE_FN_DEF(void, update_hotkey_value(hotkey_update upd, hotkey_info v));
MAKE_TEMPLATE_FN_DEF(void, preserve_hotkey_state());
MAKE_TEMPLATE_FN_DEF(void, restore_hotkey_state());

#undef MAKE_TEMPLATE_FN_DEF
