#include <gui/controls/window.h>
#include <gui/gui.h>
#include <gui/popups/search_popup.h>

GUI_NAMESPACE;

void window::render()
{
	const auto r = area_abs();
	const auto r_header = r.height(50.f);
	const auto r_toolbar = r.height(28.f).margin_top(r.height() - 28.f);

	auto &d = draw.layers[ctx->content_layer];
	d->g.anti_alias = true;

	// shadow, background
	d->add_shadow_rect(r, 24.f, true, .1f);
	d->add_rect_filled_rounded(r, colors.bg_bottom, 4.f);

	// background texture per theme: diagonal stripes, recoloured stripes, or animated hearts
	auto bg = ctx->res.general.pattern;
	vec2 tile{5.f, 5.f};

	switch (colors.theme_id)
	{
	case 1: // pride
		bg = ctx->res.general.pattern_pride;
		break;
	case 2: // dark
		bg = ctx->res.general.pattern_dark;
		break;
	case 3: // hearts
		bg = ctx->res.general.hearts;
		// draw the heart tile at half scale so the hearts read smaller, like the reference
		tile = ctx->res.general.hearts_tile_size * 0.5f;
		break;
	default:
		break;
	}

	if (bg && bg->obj)
	{
		// hearts drift down and slightly to the left
		static vec2 heart_off{};
		if (colors.theme_id == 3)
		{
			// the drift direction slowly rotates counterclockwise, so the hearts
			// travel along a wide circle (speed in px/s, turn rate in rad/s)
			constexpr float drift_speed = 34.f, turn_rate = 0.04f;
			static float drift_angle{};
			drift_angle = std::fmod(drift_angle + draw.frame_time * turn_rate, 6.2831853f);

			// screen y points down, so -sin gives a counterclockwise turn on screen
			const vec2 dir{std::cos(drift_angle), -std::sin(drift_angle)};
			heart_off.x -= draw.frame_time * drift_speed * dir.x / tile.x;
			heart_off.y -= draw.frame_time * drift_speed * dir.y / tile.y;

			heart_off.x -= std::floor(heart_off.x);
			heart_off.y -= std::floor(heart_off.y);
		}
		else
			heart_off = {};

		d->g.texture = bg->obj;
		d->g.uv_rect = rect{heart_off.x, heart_off.y, heart_off.x + r.width() / tile.x, heart_off.y + r.height() / tile.y};
		d->add_rect_filled_rounded(r, color::white(), 4.f);
		d->g.texture = {};
		d->g.uv_rect = {};
	}

	// header/toolbar are window chrome: in the white theme they stay in the light
	// background family while sections (groups) use the darker bg_block gray
	const auto chrome_col = colors.theme_id == 1 ? colors.bg_block_light : colors.bg_block;

	// header background
	d->add_rect_filled_rounded(r_header, chrome_col, 4.f, rnd_t);
	// thin separator shadows: the reference only has a soft ~5px fade at each edge
	d->add_shadow_line(r_header.margin_top(50.f).height(5.f), shadow_down, 0.2f);

	// toolbar background
	d->add_rect_filled_rounded(r_toolbar, chrome_col, 4.f, rnd_b);
	d->add_shadow_line(r_toolbar.margin_bottom(5.f).height(5.f), shadow_up, 0.2f);
	d->g.anti_alias = {};

	// FATALITY text
	const auto fnt = draw.fonts[GUI_HASH("gui_title")];
	vec2 text_pos{19.f, 15.125f};

	const auto off_x = cosf(glitch->value);
	const auto off_y = sinf(glitch->value);

	d->font = fnt;
	// pride shows a clean title; the chromatic glitch is for the other themes
	if (colors.theme_id != 1)
	{
		d->add_text(r.tl() + text_pos + vec2{off_x, off_y}, XOR("FATALITY"), color(255, 79, 94)); // #ff4f5e, recovered theme table
		d->add_text(r.tl() + text_pos + (vec2{off_x, off_y} * -1.f), XOR("FATALITY"), color(0, 0, 255));
	}
	d->add_text(r.tl() + text_pos, XOR("FATALITY"), colors.text);

	// user information
	constexpr float avatar_radius = 18.f;
	const auto avatar_pos = r.tr() + vec2{-38.f, 25.f};
	if (ctx->user.avatar)
	{
		d->g.texture = ctx->user.avatar->obj;
		d->add_rect_filled(rect{avatar_pos - vec2{avatar_radius, avatar_radius}, avatar_pos + vec2{avatar_radius, avatar_radius}}, color::white());
		d->g.texture = {};
	}
	else
	{
		d->add_circle_filled(avatar_pos, avatar_radius, colors.bg_bottom);
		d->add_circle_filled_multicolor(avatar_pos, avatar_radius, {color::black().a(.25f), color::black_transparent()});
	}

	const auto mf = draw.fonts[GUI_HASH("gui_main")];
	const auto expiry_sz = mf->get_text_size(ctx->user.active_until);

	// username is semibold in the reference; the expiry line stays medium
	d->font = draw.fonts[GUI_HASH("gui_bold")];
	d->add_text(r.tr() + vec2{-59.f, 14.f}, ctx->user.username, colors.text, text_params::with_h(align_right));
	d->font = mf;
	d->add_text(r.tr() + vec2{-59.f, 26.f}, ctx->user.active_until, colors.accent, text_params::with_h(align_right));
	d->add_text(
		r.tr() + vec2{-59.f, 26.f} - vec2{expiry_sz.x + 3.f, 0.f}, std::string(ctx->user.is_alpha ? XOR("alpha - ") : XOR("")) + XOR("expires:"), colors.text_dark,
		text_params::with_h(align_right));

	glitch->animate();
	control_container::render();
}

void window::on_mouse_down(char key)
{
	control_container::on_mouse_down(key);
	if (!allow_move)
		return;

	if (key == m_left)
	{
		is_caring_about_hover = false;
		if (ctx->top_sort != sort)
			sort = ++ctx->top_sort;

		lock_input();
	}
}

void window::on_mouse_up(char key)
{
	control_container::on_mouse_up(key);
	if (!allow_move)
		return;

	if (key == m_left)
	{
		is_caring_about_hover = true;
		unlock_input();
	}
}

void window::on_mouse_move(const vec2 &p, const vec2 &d)
{
	control_container::on_mouse_move(p, d);
	if (!allow_move || !ctx->active_popups.empty())
		return;

	if (is_locked_by_me())
	{
		const auto temp_pos = pos + d;
		if (is_out_of_clip(temp_pos))
			return;

		pos = temp_pos;
	}
}

void window::add(const std::shared_ptr<control> &c)
{
	control_container::add(c);
}
