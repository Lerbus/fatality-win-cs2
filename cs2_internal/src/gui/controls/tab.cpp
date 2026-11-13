#include <gui/controls/tab.h>
#include <gui/controls/tabs_layout.h>

GUI_NAMESPACE;

// idle (unselected, unhovered) tab tone: hearts uses a mid gray like the reference
static color tab_idle_col()
{
	return colors.theme_id == 3 ? color(128, 128, 128) : colors.text_mid;
}

void tab::render()
{
	// a theme switch leaves col_anim holding the previous palette's colour;
	// snap it straight to the new theme's selected/idle colour without fading
	if (last_theme != colors.theme_id)
	{
		last_theme = colors.theme_id;
		const auto c = is_selected ? colors.accent : tab_idle_col();
		col_anim->value = col_anim->start = col_anim->end = c;
	}

	line_anim->animate();
	col_anim->animate();

	control::render();
	if (!is_visible)
		return;

	auto &d = draw.layers[ctx->content_layer];

	// selected tab gets a rounded pill behind it
	if (is_selected)
	{
		// neutral lighter pill (matches the reference), anti-aliased so the corners stay smooth
		d->g.anti_alias = true;
		d->add_rect_filled_rounded(rect(pos_abs() + vec2{0.f, 12.f}).size({area(true).width(), 27.f}).floor(), colors.bg_block_overlay, 3.f);
		d->g.anti_alias = false;
	}

	// disabled tabs (legit, lua) stay dimmed in every theme - they are intentionally unfinished
	const bool dim = is_disabled;
	const auto idle_col = col_anim->value;
	const auto icon_col = dim ? colors.text_disabled : idle_col;
	// selected label stays text-coloured in every theme; only the icon takes the accent
	const auto txt_col = dim ? colors.text_disabled : (is_selected ? colors.text : idle_col);

	if (icon && icon->obj)
	{
		d->g.texture = icon->obj;
		d->add_rect_filled(rect(pos_abs() + vec2(7.f, 17.f)).size({15.f, 15.f}), icon_col);
		d->g.texture = {};

		if (is_highlighted)
		{
			d->g.anti_alias = true;
			d->add_circle_filled(pos_abs() + vec2(20.f, 20.f), 4.f, colors.bg_block);
			d->add_circle_filled(pos_abs() + vec2(20.f, 20.f), 2.5f, colors.accent);
			d->g.anti_alias = {};
		}
	}

	// tab labels are semibold in the reference (stroke ~2.7px)
	d->font = draw.fonts[GUI_HASH("gui_bold")];
	// only the icon changes to accent on selection; the label stays text-colored.
	d->add_text(pos_abs() + vec2(icon ? 25.f : 5.f, size.y * 0.5f + 3.f), text, txt_col, text_params::with_v(align_center));
}

void tab::on_mouse_down(char key)
{
	if (key == m_left)
	{
		const auto p = get_parent()->as<tabs_layout>();
		if (!p || is_selected)
			return;

		ctx->do_tick_sound();
		p->update_selected(shared_from_this());
		is_highlighted = false;

		lock_input();
	}
}

void tab::on_mouse_up(char key)
{
	if (key == m_left)
		unlock_input();
}

void tab::on_mouse_enter()
{
	if (is_disabled)
		return;

	if (!is_selected)
		col_anim->direct(colors.text);
}

void tab::on_mouse_leave()
{
	if (is_disabled)
		return;

	if (!is_selected)
		col_anim->direct(tab_idle_col());
}

void tab::unselect()
{
	is_selected = false;
	col_anim->direct(tab_idle_col());
	line_anim->direct(0.f);

	if (container)
		container->set_visible(false);

	//decide_movement();
}

void tab::select()
{
	is_selected = true;
	// selected tab (icon and label) is tinted with the accent colour
	col_anim->direct(colors.accent);
	line_anim->direct(1.f);

	if (container)
		container->set_visible(true);

	//decide_movement();
}

void tab::on_first_render_call()
{
	container = ctx->find(container_id);
	if (container)
	{
		container->set_breadcrumb(text);
		container->as<layout>()->nav_tab = shared_from_this();
	}

	if (!size_to_parent_w)
	{
		size.x += draw.fonts[GUI_HASH("gui_bold")]->get_text_size(text).x;
		if (icon)
			size.x += 23.f;
	}
}

void tab::decide_movement()
{
	invert_movement = !is_selected ? (input.cursor().x > pos_abs().x) : (input.cursor().x < get_parent()->as<tabs_layout>()->last_deselected_pos.x);
}
