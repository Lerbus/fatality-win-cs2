#include <menu/menu.h>

#include "game/cfg.h"
#include "menu/macros.h"
#include "menu/tab_helpers.h"

using namespace evo::gui;
using namespace evo::gui::helpers;
using namespace evo::ren;

namespace menu::group
{
	void skins_options(std::shared_ptr<layout> &e)
	{
		menu::ui::row(e, XOR("skins>options>seed"), XOR("Seed"), {MAKE("skins>options>seed", slider<int>, 0, 1000, cfg.menu.seed)});
		menu::ui::row(e, XOR("skins>options>wear"), XOR("Wear"), {MAKE("skins>options>wear", slider<int>, 0, 100, cfg.menu.wear, XOR("%d%%"))});
		menu::ui::row(
			e, XOR("skins>options>stattrak"), XOR("StatTrak"),
			{MAKE("skins>options>stattrak", slider<int>, -1, 10000, cfg.menu.stat_trak, XOR("%d kills"))});
		menu::ui::row(e, XOR("skins>options>equip-ct"), XOR("Equip CT"), {MAKE("skins>options>equip-ct", checkbox, cfg.menu.equip_ct)});
		menu::ui::row(e, XOR("skins>options>equip-t"), XOR("Equip T"), {MAKE("skins>options>equip-t", checkbox, cfg.menu.equip_t)});

		e->add(
			make_inlined(
				GUI_HASH("skins_add_save"),
				[](auto buttons)
				{
					buttons->add(MAKE("skins>options>save", button, XOR("Save")));
					buttons->add(MAKE("skins>options>add", button, XOR("Add")));
				}));
	}
	void skins_tab(std::shared_ptr<layout> &e)
	{
		const auto area = MAKE("skins>tab", layout, vec2{0.f, 0.f}, vec2{820.f, 482.f}, s_none);
		area->make_not_clip();
		// page-level layout never scrolls; only groups (smaller frames) get a scrollbar
		area->disable_scroll = true;
		// groups end exactly at the area's 482px bottom so nothing overflows
		area->add(make_groupbox(XOR("skins>options"), XOR("OPTIONS"), vec2{0.f, 287.f}, vec2{264.f, 195.f}, skins_options));
		area->add(make_groupbox(XOR("skins>agents"), XOR("AGENTS"), vec2{556.f, 350.f}, vec2{264.f, 132.f}, [](const std::shared_ptr<layout> &) { }));
		e->add(area);
	}
} // namespace menu::group
