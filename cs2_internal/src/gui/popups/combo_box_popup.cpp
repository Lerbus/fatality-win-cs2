#include <gui/popups/combo_box_popup.h>
#include <gui/controls/list.h>

GUI_NAMESPACE;

void combo_box_popup::on_first_render_call()
{
	popup::on_first_render_call();

	const auto target_size = custom_size.value_or(vec2{160.f, 100.f});

	// create list
	const auto l = std::make_shared<list>(id + 1, combo->value, vec2{}, target_size);
	l->allow_multiple = combo->allow_multiple;
	l->callback = combo->callback;
	l->universal_callbacks = combo->universal_callbacks;
	l->legacy_mode = combo->legacy_mode;

	// enable auto-close if combo is not multi select
	if (!combo->allow_multiple)
	{
		l->callback = [this](bits &v)
		{
			if (combo->callback)
				combo->callback(v);

			for (const auto &[_, cbk_list] : combo->universal_callbacks)
			{
				for (const auto &cbk : cbk_list)
					cbk();
			}

			// close ourselves, not whatever popup happens to be on top
			close();
		};
	}

	// copy controls to list
	combo->copy(l);

	// force first render call to stack items and reset to update colors
	l->do_first_render_call();
	l->reset();

	// add to our container
	add(l);

	// set proper size: no scrolling, the popup grows to show every option
	size = {target_size.x, l->calc_max_y()};
	l->size = size;

	pos = calc_perfect_position(pos);

	// set this as an active popup
	ctx->active_popups.emplace_back(as<popup>());
	ctx->top_sort = sort;
}

void combo_box_popup::render()
{
	if (!is_visible)
		return;

	begin_render();

	auto &d = draw.layers[ctx->content_layer];
	d->add_shadow_rect(area_abs(), 8.f, true);

	popup::render();
}
