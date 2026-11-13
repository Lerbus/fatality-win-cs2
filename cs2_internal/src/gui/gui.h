#ifndef GUI_GUI_H
#define GUI_GUI_H

#include <memory>
#include <stack>

#include <ren/renderer.h>
#include <ren/adapter.h>
#include <ren/types/texture.h>
#include <ren/misc.h>
#include <ren/types/animator.h>

#include <gui/misc.h>
#include <gui/input.h>
#include <gui/container.h>
#include <gui/controls/control_container.h>
#include <gui/controls/popup.h>
#include <gui/values.h>

#undef min
#undef max

#define GUI_NAMESPACE using namespace evo; using namespace gui; using namespace ren
#define MAKE_COLOR(n, r, g, b) ren::color n = ren::color(r, g, b)

#include <macros.h>

namespace evo::ren
{
	void add_with_blur(std::shared_ptr<layer> &d, const rect &area,
					   const std::function<void(std::shared_ptr<layer> &)> &fn);
}

namespace evo::gui
{
	enum gui_layers : char
	{
		l_back = 32,
		l_mid,
		l_hover,
		l_popup_back,
		l_popup_mid,
		l_popup_top,
		l_top,
		l_last
	};

	enum blur_quality
	{
		blur_full,
		blur_half,
		blur_none,
		blur_max,
	};

	struct color_list
	{
		// recovered fatality2026 theme table (see theme_colors.hpp / theme_default.json)
		MAKE_COLOR(accent, 239, 9, 95); // #ef095f
			MAKE_COLOR(ui_accent, 239, 9, 95); // combo glyph tint (dark in the white theme, accent elsewhere)
		// purple theme, sampled from the reference screenshot
		MAKE_COLOR(text, 245, 245, 245); // #f5f5f5
		MAKE_COLOR(text_light, 215, 215, 215); // #d7d7d7
		MAKE_COLOR(text_mid, 98, 90, 136); // #625a88
		MAKE_COLOR(text_dark, 98, 90, 136); // #625a88
		// dimmed grayish tone for disabled tabs (legit, lua)
		MAKE_COLOR(text_disabled, 58, 52, 92); // #3a345c, muted purple like the reference
		MAKE_COLOR(bg_bottom, 17, 15, 35); // #110f23, recovered stripe tile base
		// control wells (checkbox, combo, slider, text input, button) - separate from the window body
		MAKE_COLOR(bg_input, 17, 15, 36); // #110f24, one step off bg_bottom so theme remapping keeps them distinct
		MAKE_COLOR(bg_block, 25, 21, 52); // #191534
		MAKE_COLOR(bg_block_overlay, 30, 25, 62); // #1e193e
		MAKE_COLOR(bg_block_light, 30, 25, 62); // #1e193e
		MAKE_COLOR(bg_odd, 25, 21, 52); // #191534
		MAKE_COLOR(bg_even, 17, 15, 35); // #110f23, recovered stripe tile base
		MAKE_COLOR(outline, 44, 37, 77); // #2c254d
		MAKE_COLOR(outline_light, 44, 37, 77); // #2c254d
		MAKE_COLOR(warning, 214, 105, 38); // #d66926
		MAKE_COLOR(danger, 245, 66, 66); // #f54242
		MAKE_COLOR(success, 143, 255, 102); // #8fff66

		// 0 default, 1 pride, 2 dark, 3 hearts (mirrors cfg_t::menu_theme)
		int theme_id{};

		void set_theme(int id);

		// slider fill colour: accent everywhere except pride, where each slider gets
		// its own rainbow hue so they read as "mixed" across the menu
		[[nodiscard]] ren::color slider_fill(uint64_t id) const;
	};

	class context : public container
	{
	public:
		context(float text_size = 14.5f); // Figtree Medium 14.5, fitted to reference text widths

		// refreshes the input
		bool refresh(uint32_t msg, uint32_t w, uint32_t l);

		// renders controls
		void render();

		// resets control's state
		void reset();

		// tracks animation to auto-update when accent color is changed
		void track_accent_anim(const std::weak_ptr<ren::anim_color> &a);
		void track_accent_anim(const std::weak_ptr<anim_float_color> &a);

		void preserve_hotkey_state();
		void restore_hotkey_state();

		void remove(const std::shared_ptr<control> &c) override;
		void remove(uint64_t id) override;

		void do_tick_sound()
		{
			if (tick_sound_callback)
				tick_sound_callback();
		}

		void add_chord(const chord &ch);

		char content_layer{l_back}; // current layer for controls
		char scrollbar_layer{l_mid}; // current layer for scroll bars

		std::shared_ptr<control> active{}; // current active element
		std::vector<std::shared_ptr<popup>> active_popups{}; // current active popups
		int top_sort{}; // current top most sort

		std::function<void()> tick_sound_callback{};
		std::unordered_map<uint64_t, std::weak_ptr<control>> hotkey_list{}; // used to process hotkeys separately
		std::unordered_map<uint64_t, std::weak_ptr<control>> tooltip_list{};
		std::unordered_map<uint64_t, std::weak_ptr<control>> global_list{}; // for quick ass search! 

		[[nodiscard]] uint64_t get_last_hovered() const { return hover_stack.empty() ? 0ull : hover_stack.top(); }

		std::stack<uint64_t> hover_stack{};
		value_param<bool> show_ids{};
		value_param<bits> blur_quality{1 << blur_full};

		struct resources_t
		{
			struct
			{
				std::shared_ptr<ren::texture> pattern{};
				std::shared_ptr<ren::texture> pattern_dark{};
				std::shared_ptr<ren::texture> pattern_pride{};
				std::shared_ptr<ren::texture> hearts{};
				ren::vec2 hearts_tile_size{5.f, 5.f};
				// pride sliders: one shared pink -> blue ramp sampled across the full width
				std::shared_ptr<ren::texture> slider_gradient{};
				std::shared_ptr<ren::texture> logo_head{};
				std::shared_ptr<ren::texture> logo_stripes{};
				std::shared_ptr<ren::texture> pattern_group{};
				std::shared_ptr<ren::texture> loader{};
				std::shared_ptr<ren::texture> cursor{};
			} general;

			struct
			{
				std::shared_ptr<ren::texture> notify_clear{};
				std::shared_ptr<ren::texture> remove{};
				std::shared_ptr<ren::texture> add{};
				std::shared_ptr<ren::texture> up{};
				std::shared_ptr<ren::texture> down{};
				std::shared_ptr<ren::texture> copy{};
				std::shared_ptr<ren::texture> paste{};
				std::shared_ptr<ren::texture> alert{};
				std::shared_ptr<ren::texture> search{};
				std::shared_ptr<ren::texture> settings{};
				std::shared_ptr<ren::texture> bug{};

				std::unordered_map<uint8_t, std::shared_ptr<ren::texture>> keys{};
			} icons;
		} res;

		struct user_t
		{
			std::shared_ptr<ren::texture> avatar{};
			std::string username{};
			std::string active_until{};
			bool is_alpha{};
		} user;

		bool should_update_old_value{};
		bool should_render_cursor{false};
		bool should_process_hotkeys{true};

	private:
		std::mutex accents_mutex{};

		std::vector<std::weak_ptr<ren::anim_color>> accents{};
		std::vector<std::weak_ptr<anim_float_color>> accents_float{};
		ren::color accent_prev{};

		std::vector<chord_info> chords;

		void process_hotkeys();
		void process_chords();
	};

	class context_debug
	{
	public:
		// renders basic debug info
		void render();

		// renders control's info
		void render(const std::shared_ptr<control> &c);

		ren::color fill = ren::color(255, 74, 122);
		ren::color fill_active = ren::color(255, 207, 74);
		bool enabled = false;
	};

	inline context_input input{}; // input processor
	inline context_debug debug{}; // debug overlay
	inline color_list colors{}; // color list

	inline std::unique_ptr<context> ctx{}; // main context
}

#endif //GUI_GUI_H
