#pragma once

#include <gui/gui.h>
#include <gui/controls.h>
#include <gui/helpers.h>

namespace menu
{
	namespace tools
	{
		using namespace evo;
		using namespace evo::ren;

		std::shared_ptr<evo::gui::control> make_stacked_groupboxes(uint64_t id, const vec2 &size, const std::vector<std::shared_ptr<evo::gui::control>> &groups);
	}

	namespace group
	{
		using namespace evo::gui;

		void rebuild_rage_layout(bool skip_adjust = false);
		void rebuild_legit_layout(bool skip_adjust = false);

		void rage_tab(std::shared_ptr<layout> &e);
		void visuals_tab(std::shared_ptr<layout> &e);
		void misc_tab(std::shared_ptr<layout> &e);
		void skins_tab(std::shared_ptr<layout> &e);
	}

	class menu_t
	{
	public:
		void init();
		void toggle();
		void update_dpi_scale();
		bool is_open();
		void finalize();
		bool enable_cursor = false;
		std::weak_ptr<evo::gui::window> main_wnd{};
		std::atomic_bool skins_loaded, skins_ready, full_cycle_reset, font_reset, font_was_reset;
		std::mutex render_mtx, scripts_mtx;
		bool finalized{};
	};

	extern menu_t men;
}
