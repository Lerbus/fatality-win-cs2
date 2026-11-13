#include <gui/controls/child_tab.h>

GUI_NAMESPACE;

void child_tab::render()
{
	control::render();
	if (!is_visible)
		return;

	auto &d = draw.layers[ctx->content_layer];

	const auto r = area_abs();

	// a theme switch leaves an holding the previous palette's colour; snap to the new one
	if (last_theme != colors.theme_id)
	{
		last_theme = colors.theme_id;
		const auto c = is_selected ? colors.text : colors.text_dark;
		an->value = an->start = an->end = c;
	}

	an->animate();
	of->animate();

	const auto c = an->value;
	const auto o = of->value;
	d->font = draw.fonts[is_selected ? GUI_HASH("gui_bold") : GUI_HASH("gui_main")];

	if (is_horizontal)
	{
		d->add_text(r.center() - vec2{2.f, 0.f}, text, c, text_params::with_vh(align_center, align_center));
		if (o > 0.f)
			d->add_rect_filled(r.height(2.f).width((r.width() - 3.f) * o).margin_top(size.y - 1.5f), colors.accent);
	}
	else
	{
		d->add_text(r.tl() + vec2(9.f, size.y * 0.5f + 1.f), text, c, text_params::with_v(align_center));
		if (o > 0.f)
			d->add_rect_filled(rect(r.tl() + vec2{1.f, 0.f}).height(r.height() * o).width(2.f), colors.accent);
	}
}

void child_tab::on_mouse_enter()
{
	tab::on_mouse_enter();
	an->direct(colors.text);
}

void child_tab::on_mouse_leave()
{
	tab::on_mouse_leave();
	an->direct(is_selected ? colors.text : colors.text_dark);
}

void child_tab::on_first_render_call()
{
	tab::on_first_render_call();

	if (!size_to_parent_w)
	{
		size = draw.fonts[GUI_HASH("gui_bold")]->get_text_size(text);
		size.y += 8.f;
		size.x += 8.f;
	}
	else
		size.y = is_horizontal ? (draw.fonts[GUI_HASH("gui_bold")]->get_text_size(text).y + 8.f) : 20.f;

	if (is_selected)
	{
		an->direct(colors.text);
		of->direct(1.f, 1.f);
	}
}

void child_tab::select()
{
	tab::select();
	an->direct(colors.text);
	of->direct(1.f);
}

void child_tab::unselect()
{
	tab::unselect();
	an->direct(colors.text_dark);
	of->direct(0.f);
}
