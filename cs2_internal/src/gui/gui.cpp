#include <gui/gui.h>
#include <gui/anim.h>
#include <gui/controls/checkbox.h>
#include <gui/controls/label.h>
#include <gui/controls/selectable.h>
#include <gui/controls/tab.h>
#include <utils/debug_log.h>

#include <resources/figtree_medium.h>
#include <resources/figtree_semibold.h>
#include <resources/figtree_extrabold.h>

GUI_NAMESPACE;

#ifdef _DEBUG
static std::atomic_uint64_t gui_refresh_count{};
#endif

namespace evo::ren
{
	void add_with_blur(std::shared_ptr<layer> &d, const rect &area, const std::function<void(std::shared_ptr<layer> &)> &fn)
	{
		if (gui::ctx->blur_quality->test(gui::blur_none))
			return;

		const auto ds_display = draw.display / draw.scale * .25f;
		d->g.frag_shader = draw.shaders[GUI_HASH("blur_f")]->obj;
		d->g.texture = draw.adapter->get_back_buffer_downsampled();
		d->g.uv_rect = {area.tl() * .25f / ds_display, area.br() * .25f / ds_display};
		d->g.only_downsampled = true;
		fn(d);

		d->g.chained_call = true;
		if (gui::ctx->blur_quality->test(gui::blur_full))
			fn(d);

		d->g.only_downsampled = d->g.chained_call = {};
		d->g.frag_shader = d->g.texture = {};
		d->g.uv_rect = {};
	}
} // namespace evo::ren

void color_list::set_theme(int id)
{
	const color_list prev = *this;

	// start from the default palette (the header initializers), then apply
	// only the values that differ for the selected theme
	*this = color_list{};
	theme_id = id;

	switch (id)
	{
	case 1: // pride - white theme, sliders use mixed rainbow colours
		// sampled from pride.png (median of 5x5 patches)
		// values from color_palette_package/exact_palettes/pride_all_exact_colors.csv
		bg_bottom = {210, 213, 217}; // #d2d5d9 window body (stripe base)
		bg_block = {227, 229, 232}; // #e3e5e8 sections (59.7% of pixels)
		bg_block_light = {227, 229, 233}; // header / toolbar, same tone as sections
		bg_block_overlay = {242, 243, 245}; // #f2f3f5 selected tab pill
		bg_input = {241, 246, 253}; // #f1f6fd input wells
		bg_odd = {229, 231, 234}; // #e5e7ea
		bg_even = {228, 230, 233}; // #e4e6e9
		text = {68, 68, 69}; // #444445
		text_light = {95, 96, 96}; // #5f6060
		text_mid = {123, 124, 125}; // #7b7c7d
		text_dark = {119, 119, 120}; // #777778
		text_disabled = {162, 163, 165}; // #a2a3a5
		outline = {222, 226, 231}; // #dee2e7 section rim
		outline_light = {232, 236, 241}; // #e8ecf1 input well rim
		accent = {247, 133, 164}; // #f785a4
		ui_accent = {163, 217, 237}; // #a3d9ed combo glyph
		break;
	case 2: // dark - values from dark_all_exact_colors.csv
		bg_bottom = {20, 20, 20}; // #141414 window body (stripe base)
		bg_block = {28, 28, 28}; // #1c1c1c sections (64.6% of pixels)
		bg_block_light = {28, 28, 29}; // header / toolbar
		bg_block_overlay = {44, 44, 44}; // #2c2c2c selected tab pill
		bg_input = {22, 22, 22}; // #161616 input wells
		bg_odd = {35, 35, 35}; // #232323
		bg_even = {31, 31, 31}; // #1f1f1f
		text = {245, 245, 245}; // #f5f5f5
		text_light = {220, 220, 220}; // #dcdcdc
		text_mid = {164, 164, 164}; // #a4a4a4
		text_dark = {96, 96, 96}; // #606060
		text_disabled = {68, 68, 68}; // #444444
		outline = {38, 38, 38}; // #262626 1px section border
		outline_light = {22, 22, 23}; // checkbox, same as input wells
		accent = {255, 79, 94}; // #ff4f5e
		ui_accent = {234, 234, 234}; // #eaeaea combo glyph
		break;
	case 3: // hearts - values from hearts_all_exact_colors.csv
		bg_bottom = {17, 17, 17}; // #111111 body under the heart tile
		bg_block = {22, 22, 22}; // #161616 sections (27.9% of pixels)
		bg_block_light = {22, 22, 23}; // header / toolbar
		bg_block_overlay = {62, 22, 34}; // #3e1622 selected tab pill - softer, reads as a faint red wash over the header
		bg_input = {19, 19, 19}; // #131313 input wells
		bg_odd = {27, 27, 27}; // #1b1b1b
		bg_even = {26, 26, 26}; // #1a1a1a
		text = {255, 255, 255}; // #ffffff
		text_light = {216, 216, 216}; // #d8d8d8
		text_mid = {200, 200, 200}; // #c8c8c8
		text_dark = {82, 82, 82}; // #525252
		text_disabled = {87, 87, 87}; // #575757
		outline = {35, 35, 35}; // #232323
		outline_light = {33, 33, 33}; // #212121
		accent = {255, 36, 94}; // #ff245e
		ui_accent = accent;
		break;
	default:
		break;
	}

	// remap colours captured by controls when they were constructed
	if (!ctx)
		return;

	auto remap = [&](color c) -> color
	{
		if (c == prev.text) return text;
		if (c == prev.text_light) return text_light;
		if (c == prev.text_mid) return text_mid;
		if (c == prev.text_dark) return text_dark;
		if (c == prev.text_disabled) return text_disabled;
		if (c == prev.accent) return accent;
		if (c == prev.ui_accent) return ui_accent;
		if (c == prev.warning) return warning;
		if (c == prev.outline) return outline;
		if (c == prev.outline_light) return outline_light;
		if (c == prev.bg_bottom) return bg_bottom;
		if (c == prev.bg_input) return bg_input;
		if (c == prev.bg_block) return bg_block;
	 if (c == prev.bg_block_light) return bg_block_light;
		if (c == prev.bg_block_overlay) return bg_block_overlay;
		if (c == prev.bg_odd) return bg_odd;
		if (c == prev.bg_even) return bg_even;
		if (c == prev.danger) return danger;
		if (c == prev.success) return success;
		return c;
	};

	for (auto &[control_id, weak] : ctx->global_list)
	{
		const auto c = weak.lock();
		if (!c)
			continue;

		switch (c->type)
		{
		case ctrl_label:
			if (const auto l = c->as<label>())
				l->col = remap(l->col);
			break;
		case ctrl_checkbox:
			c->as<checkbox>()->reset_internal();
			break;
		case ctrl_tab:
		case ctrl_child_tab:
			if (const auto t = c->as<tab>())
			{
				if (t->is_selected)
					t->select();
				else
					t->unselect();
			}
			break;
		case ctrl_selectable:
			if (const auto s = c->as<selectable>(); s)
			{
				if (s->is_selected)
					s->select();
				else
					s->unselect();
			}
			break;
		default:
			break;
		}
	}
}

ren::color color_list::slider_fill(uint64_t id) const
{
	if (theme_id == 1)
	{
		// pride: each slider gets its own rainbow hue
		return accent.hsv(static_cast<uint16_t>((id ^ (id >> 16)) % 360), 0.75f, 0.9f);
	}
	return accent;
}

context::context(float text_size)
{
	// cache layers
	for (int i{l_back}; i <= l_last; ++i)
		draw.create_layer(i);

	draw.create_layer(50);
	draw.layers[50]->skip_dpi = true;

	// create fonts
	char win_dir_arr[256]{};
	GetSystemWindowsDirectoryA(win_dir_arr, 256);

	std::string win_dir{win_dir_arr};
	draw.fonts[GUI_HASH("gui_debug")] = std::make_shared<font>((win_dir + XOR("/fonts/verdana.ttf")).c_str(), 14.f, 0, 0, 0x45F);
	draw.fonts[GUI_HASH("gui_title")] = std::make_shared<font>(figtree_extrabold.data(), figtree_extrabold.size(), 22.5f);
	draw.fonts[GUI_HASH("gui_main")] = std::make_shared<font>(figtree_medium.data(), figtree_medium.size(), text_size, 0, 0, 0x45F);
	draw.fonts[GUI_HASH("gui_bold")] = std::make_shared<font>(figtree_semibold.data(), figtree_semibold.size(), text_size, 0, 0, 0x45F);

	// gaussian blur over the scene capture. constants come from the pixel cbuffer
	// (projection_matrix, tex = capture resolution, time, alpha) - see
	// pixel_constant_buffer / adapter_dx11::push_pixel_constants.
	draw.shaders[GUI_HASH("blur_f")] = std::make_shared<shader>(
		XOR(
			"cbuffer cb:register(b0){float4x4 projection_matrix;float2 tex;float time;float alpha;};"
			"struct PS_INPUT{float4 pos:SV_POSITION;float4 col:COLOR0;float2 uv:TEXCOORD0;};"
			"SamplerState sampler0;Texture2D texture0;"
			"float4 main(PS_INPUT input):SV_Target{"
			"float radius=2.0;float2 invSize=1.0/tex.xy;float weight=0.0;float4 color=0.0;"
			"for(float x=-radius;x<=radius;x+=1.0){for(float y=-radius;y<=radius;y+=1.0){"
			"float2 coord=input.uv+float2(x,y)*invSize;float w=exp(-((x*x+y*y)/(2.0*radius*radius)));"
			"color+=texture0.Sample(sampler0,coord)*w;weight+=w;}}"
			"color/=weight;return float4(input.col.rgb*color.rgb,input.col.a);}"), shader_frag);

	accent_prev = colors.accent;
}

bool context::refresh(uint32_t msg, uint32_t w, uint32_t l)
{
	#ifdef _DEBUG
		const auto refresh_count = ++gui_refresh_count;
		if (refresh_count % 1000 == 0)
			utils::debug::log(utils::debug::level::trace, "GUI", "refresh count=%llu msg=0x%X active=%p popups=%zu",
				static_cast<unsigned long long>(refresh_count), msg, active.get(), active_popups.size());
	#endif
	// take input
	const auto r = input.on_input(msg, w, l);

	// update whichever have hotkey
	const auto is_active_hotkey = active && active->type == ctrl_hotkey;
	if (input.did_process_key() && ctx->should_process_hotkeys && !is_active_hotkey)
	{
		process_hotkeys();
		process_chords();
	}

	// no point to process everything if nothing we want was captured
	if (!input.did_process_key() && !input.did_cursor_move() && !input.did_wheel_move())
	{
		input.on_input_end();
		return r;
	}

	// Wait for the current frame instead of dropping/defering this Windows message.
	// A failed try_lock left the click pending until another message arrived, which
	// made buttons and sliders appear to respond late (often only after moving).
	const std::lock_guard lock(container_mutex);

	if (!active_popups.empty())
	{
		utils::debug::log(utils::debug::level::trace, "GUI", "top popup: id=0x%llX visible=%d",
			static_cast<unsigned long long>(active_popups.back()->id), active_popups.back()->is_visible);
		auto &last = active_popups.back();
		if (!last->area_abs().contains(input.cursor()) && input.is_mouse_clicked(m_left))
		{
			last->close();
			input.on_input_end();
			return r;
		}
	}

	// process controls
	for_each_control_ordered(
		[&](std::shared_ptr<control> &c)
		{
			if ((c->is_taking_input || c->is_container) && c->is_visible)
			{
				c->refresh();

				// process only topmost window
				if (c->is_window && c->is_mouse_on_me)
				{
					// reset tooltips
					for (const auto &[id, cw] : tooltip_list)
					{
						const auto cr = cw.lock();
						if (!cr)
							continue;

						if (const auto wnd = cr->find_associated_window(); wnd && wnd->id != c->id)
						{
							cr->refresh();
							tooltip_list[id] = {};
						}
					}

					return true;
				}
			}

			return false;
		});

	// call refresh if active element went invisible
	if (active && !active->is_visible)
		active->refresh();

	input.on_input_end();
	return r;
}

void context::render()
{
	// debug rendering
	debug.render();

	// adjust accent values
	if (accents_mutex.try_lock())
	{
		if (colors.accent != accent_prev)
		{
			for (auto &i : accents)
			{
				const auto c = i.lock();
				if (c && c->end == accent_prev)
					c->direct(colors.accent);
			}

			for (auto &i : accents_float)
			{
				const auto c = i.lock();
				if (c && c->end.c == accent_prev)
					c->direct(u_fc(4.f, colors.accent));
			}

			accent_prev = colors.accent;
		}

		// purge expired accents
		for (auto i = accents.begin(); i != accents.end();)
			i->expired() ? (i = accents.erase(i)) : ++i;
		for (auto i = accents_float.begin(); i != accents_float.end();)
			i->expired() ? (i = accents_float.erase(i)) : ++i;

		accents_mutex.unlock();
	}

	const std::lock_guard lock(container_mutex);

	// queue processor
	process_add_schedule();

	for (auto &c : controls)
	{
		if (c->is_container)
			c->as<control_container>()->process_queues();
	}

	process_remove_schedule();

	// first render call
	for (auto &c : controls)
	{
		// process containers first
		if (c->is_container)
			c->as<control_container>()->raise_first_render_call();

		// then control
		c->do_first_render_call();
	}

	// render controls
	for_each_control_ordered_reverse(
		[](std::shared_ptr<control> &c)
		{
			if (c->is_visible)
				c->render();

			return false;
		});

	// render cursor, if one is available
	if (res.general.cursor && should_render_cursor)
	{
		const auto &d = draw.layers[l_last];
		d->g.texture = res.general.cursor->obj;
		d->add_rect_filled(rect(input.cursor()).size(vec2{32.f, 32.f}).translate({-12.f, -11.f}), colors.accent);
		d->g.texture = {};
	}
}

void context::reset()
{
	// reset all controls
	for_each_control(
		[](std::shared_ptr<control> &c)
		{
			c->reset();
		});
}

void context::preserve_hotkey_state()
{
	for (auto &[_, cu] : hotkey_list)
	{
		const auto c = cu.lock();
		if (!c)
			continue;

		c->preserve_hotkey_state();
	}
}

void context::restore_hotkey_state()
{
	for (auto &[_, cu] : hotkey_list)
	{
		const auto c = cu.lock();
		if (!c)
			continue;

		c->restore_hotkey_state();
	}
}

void context::track_accent_anim(const std::weak_ptr<ren::anim_color> &a)
{
	std::lock_guard abc(accents_mutex);
	accents.emplace_back(a);
}

void context::track_accent_anim(const std::weak_ptr<anim_float_color> &a)
{
	std::lock_guard abc(accents_mutex);
	accents_float.emplace_back(a);
}

void context::remove(const std::shared_ptr<control> &c)
{
	if (c->is_container)
		c->as<control_container>()->remove_all();

	container::remove(c);
}

void context::remove(uint64_t id)
{
	if (auto p = std::find_if(
		controls.begin(), controls.end(), [id](const std::shared_ptr<control> &c) -> bool
		{
			return c->id == id;
		}); p != controls.end())
	{
		const auto &c = *p;
		if (c->is_container)
			c->as<control_container>()->remove_all();
	}

	container::remove(id);
}

void context::process_hotkeys()
{
	std::vector<uint64_t> removal_queue{};
	removal_queue.reserve(hotkey_list.size());

	// refresh hotkeys
	for (const auto &[id, weak_c] : hotkey_list)
	{
		if (const auto c = weak_c.lock(); c)
			c->refresh_hotkeys();
		else
			removal_queue.emplace_back(id);
	}

	// process removal queue if some controls are gone
	if (!removal_queue.empty())
	{
		for (const auto &i : removal_queue)
			hotkey_list.erase(i);
	}
}

void context::add_chord(const chord &ch)
{
	if (ch.keys.empty())
		return;

	// test if a chord already exists.
	if (const auto i = std::ranges::find_if(
		chords, [&](const chord_info &c)
		{
			return c.ch.mods == ch.mods && std::ranges::count_if(
				c.ch.keys, [&](char k)
				{
					return std::ranges::find(ch.keys, k) != ch.keys.end();
				});
		}); i != chords.end())
		throw; // nope

	// put a new record.
	chords.emplace_back(chord_info{ch, std::vector<char>{}});
}

void context::process_chords()
{
	for (auto &[ch, lg] : chords)
	{
		if (std::ranges::count_if(ch.mods, [&](char k) { return input.is_key_down(k); }) < ch.mods.size())
		{
			// depress others.
			lg.clear();
			continue;
		}

		if (ch.keys.size() == 1)
		{
			if (input.is_key_clicked(ch.keys[0]))
				ch.on_exec();

			continue;
		}

		// this originates from a meme in ev0 loader.
		// press required ones.
		for (const auto &k : ch.keys)
		{
			if (std::ranges::find(lg, k) == lg.end())
			{
				if (input.is_key_clicked(k))
					lg.emplace_back(k);
			}
		}

		// check if all processed and depress if so.
		if (lg.size() == ch.keys.size())
		{
			ch.on_exec();
			lg.clear();
		}
	}
}

void context_debug::render()
{
	if (!enabled)
		return;

	const auto &fnt = draw.fonts[GUI_HASH("gui_debug")];
	const auto &fps_raw = draw.frame_time;
	const auto fps = fps_raw == 0.f ? 999.f : (1.f / fps_raw);

	const auto cur = input.cursor();

	char buffer[128]{};
	sprintf_s(buffer, 128, XOR("%i / gui [%.0f : %.0f]"), (int)fps, cur.x, cur.y);

	const auto r = rect({}, fnt->get_text_size(buffer) + 4.f);

	const auto &d = draw.layers[l_last];
	d->add_rect(r.translate({1.f, 1.f}), color::black());
	d->add_rect_filled(r, fill);
	d->font = fnt;
	d->add_text(r.tl() + 2.f, buffer, color::white());
}

void context_debug::render(const std::shared_ptr<control> &c)
{
	if (!enabled)
		return;

	const auto is_active = ctx->active && (ctx->active->id == c->id);
	const auto col = is_active ? fill_active : fill;

	const auto &d = draw.layers[l_last];
	if (c->is_mouse_on_me)
	{
		const auto &fnt = draw.fonts[GUI_HASH("gui_debug")];

		const auto buffer = tfm::format(
			XOR("%.0f:%.0f <%.0f:%.0f> : <%.0f:%.0f> / %s / %i"), c->pos_abs().x, c->pos_abs().y, c->pos.x, c->pos.y, c->size.x, c->size.y,
			c->id_string.empty() ? XOR("<INT ID>") : c->id_string, c->sort);

		const auto size = fnt->get_text_size(buffer);
		const auto area = rect(c->pos_abs() - vec2{0.f, size.y + 4.f}).size(size + 4.f);

		d->add_rect(c->area_abs().translate({1.f, 1.f}), color::black());
		d->add_rect(area.translate({1.f, 1.f}), color::black());
		d->add_rect_filled(area, col);
		d->font = fnt;
		d->add_text(area.tl() + 2.f, buffer, color::white());
	}

	d->add_rect(c->area_abs(), col.mod_a((is_active || c->is_mouse_on_me) ? 255 : 25));
}
