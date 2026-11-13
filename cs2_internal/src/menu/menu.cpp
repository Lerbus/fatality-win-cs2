#include "menu.h"

#include "resources/cursor.h"
#include "resources/avatar.h"
#include "resources/icon_add.h"
#include "resources/icon_clear.h"
#include "resources/icon_cloud.h"
#include "resources/icon_copy.h"
#include "resources/icon_down.h"
#include "resources/icon_legit.h"
#include "resources/icon_misc.h"
#include "resources/icon_paste.h"
#include "resources/icon_rage.h"
#include "resources/icon_refresh.h"
#include "resources/icon_search.h"
#include "resources/icon_skins.h"
#include "resources/icon_scripts.h"
#include "resources/icon_up.h"
#include "resources/icon_visuals.h"
#include "resources/icon_save.h"
#include "resources/icon_configs.h"
#include "resources/icon_keys.h"
#include "resources/icon_info.h"
#include "resources/icon_close.h"
#include "resources/icon_settings.h"
#include "resources/icon_load.h"
#include "resources/icon_import.h"
#include "resources/icon_export.h"
#include "resources/icon_delete.h"
#include "resources/icon_bug.h"
#include "resources/icon_file.h"
#include "resources/icon_autoload.h"
#include "resources/logo_head.h"
#include "resources/logo_stripes.h"
#include "resources/pattern.h"
#include "resources/pattern_dark.h"
#include "resources/pattern_pride.h"
#include "resources/hearts_tile.h"
#include "resources/pop_sound.h"
#include "resources/icon_allow_insecure.h"
#include "resources/icon_cloud_upd.h"
#include "resources/loading.h"
#include "resources/hotkey_icons.h"

#include <gui/helpers.h>
#include <gui/values.h>
#include <game/cfg.h>
#include <menu/macros.h>

#include "game/game.h"
#include "gui/popups/about_popup.h"
#include "gui/popups/search_popup.h"
#include "gui/popups/configs_popup.h"
#include "gui/popups/hotkey_overview_popup.h"
#include "gui/controls/selectable_script.h"
#include "sdk/inputsystem.h"
#include "utils/debug_log.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") // mciSendStringA for the pop sound

using namespace evo::ren;

static bool safe_cfg_save(const std::string &path)
{
	__try
	{
		cfg.save(path);
		return true;
	}
	__except (utils::debug::seh_filter("quick-save", GetExceptionInformation()))
	{
		return false;
	}
}

static bool safe_search_open(evo::gui::search_popup *popup)
{
	__try
	{
		if (!popup)
			return false;

		popup->open();
		return true;
	}
	__except (utils::debug::seh_filter("search-open", GetExceptionInformation()))
	{
		return false;
	}
}

static bool safe_configs_open(gui::popups::configs_popup *popup)
{
	__try
	{
		if (!popup)
			return false;

		popup->open();
		return true;
	}
	__except (utils::debug::seh_filter("configs-open", GetExceptionInformation()))
	{
		return false;
	}
}

void on_quicksave()
{
	utils::debug::log(utils::debug::level::trace, "CFG", "quick-save entered: menu=%d ctx=%p path=\"%s\"",
		menu::men.is_open(), evo::gui::ctx.get(), cfg.last_changed_config.c_str());

    if (!evo::gui::ctx)
	{
		utils::debug::log(utils::debug::level::warning, "CFG", "quick-save skipped: GUI context is null");
        return;
	}

#ifdef CS2_CLOUD
	VIRTUALIZER_TIGER_WHITE_START;
	if (cfg.last_changed_config.empty() || !menu::men.is_open())
	{
		utils::debug::log(utils::debug::level::warning, "CFG", "quick-save skipped: no active config or menu closed");
		return;
	}

	const auto time = std::chrono::system_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(time - cfg.last_cfg_action).count() < 1)
		return;

	cfg.last_cfg_action = time;

	std::stringstream buf;
	cfg.save(buf);

	networking::conn->send_config_upload(cfg.last_changed_config, util::base64_encode(buf.str()));

	VIRTUALIZER_TIGER_WHITE_END;
#else
	if (cfg.last_changed_config.empty() || !menu::men.is_open())
		return;

	const auto time = std::chrono::system_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(time - cfg.last_cfg_action).count() < 1)
		return;

	cfg.last_cfg_action = time;

	utils::debug::log(utils::debug::level::trace, "CFG", "quick-save writing \"%s\"", cfg.last_changed_config.c_str());
	if (!safe_cfg_save(cfg.last_changed_config))
		utils::debug::log(utils::debug::level::error, "CFG", "quick-save aborted by exception");
	else
		utils::debug::log(utils::debug::level::ok, "CFG", "quick-save completed");
#endif
}

void on_search()
{
	if (!evo::gui::ctx || !menu::men.is_open())
		return;

	const auto s = evo::gui::ctx->find(GUI_HASH("tools>search"));
	if (!s)
	{
		utils::debug::log(utils::debug::level::error, "SEARCH", "search skipped: tools>search control not found");
		return;
	}

	utils::debug::log(utils::debug::level::trace, "SEARCH", "opening search popup at %.1f,%.1f", s->pos_abs().x, s->pos_abs().y);
	const auto p = std::make_shared<evo::gui::search_popup>(evo::gui::control_id{GUI_HASH("search")}, s->pos_abs() - vec2{0.f, 200.f});
	if (!safe_search_open(p.get()))
	{
		utils::debug::log(utils::debug::level::error, "SEARCH", "search popup failed; fallback returned to menu");
		return;
	}

	utils::debug::log(utils::debug::level::ok, "SEARCH", "search popup opened");
}

namespace menu::tools
{
	std::shared_ptr<evo::gui::control> make_stacked_groupboxes(uint64_t id, const vec2 &size, const std::vector<std::shared_ptr<evo::gui::control>> &groups)
	{
		const auto l = std::make_shared<evo::gui::layout>(evo::gui::control_id{id, ""}, vec2(), size, evo::gui::s_vertical);
		l->make_not_clip()->margin = rect();

		std::shared_ptr<evo::gui::control> last{};
		for (const auto &c : groups)
		{
			c->margin = rect(0.f, 0.f, 0.f, 10.f);
			l->add(c);
			last = c;
		}

		last->size.y -= 2.f;
		last->margin = rect();
		return l;
	}
} // namespace menu::tools

namespace menu
{
	using namespace evo::gui;
	using namespace evo::gui::helpers;
	using namespace gui::popups;

	menu_t men;

	void menu_t::update_dpi_scale()
	{
		const auto dpi = cfg.misc.dpi_scale.get();
		float new_scale{};

		if (dpi.test(cfg_t::dpi_50)) new_scale = .5f;
		else if (dpi.test(cfg_t::dpi_75)) new_scale = .75f;
		else if (dpi.test(cfg_t::dpi_100)) new_scale = 1.f;
		else if (dpi.test(cfg_t::dpi_125)) new_scale = 1.25f;
		else if (dpi.test(cfg_t::dpi_150)) new_scale = 1.5f;
		else if (dpi.test(cfg_t::dpi_200)) new_scale = 2.f;
		else new_scale = std::clamp(draw.display.y, 1.f, FLT_MAX) / 1080.f;

		if (std::fabs(draw.scale - new_scale) < 0.0001f)
			return;

		draw.scale = new_scale;
		if (const auto wnd = main_wnd.lock(); wnd && wnd->is_out_of_clip(wnd->pos))
			wnd->pos = {50.f, 50.f};
		font_reset = true;
	}

	void menu_t::init()
	{
		ctx->res.general.logo_head = std::make_shared<texture>(data::logo_head, sizeof(data::logo_head));
		ctx->res.general.logo_stripes = std::make_shared<texture>(data::logo_stripes, sizeof(data::logo_stripes));
		ctx->res.general.cursor = std::make_shared<texture>(data::cursor, sizeof(data::cursor));
		ctx->res.general.loader = std::make_shared<texture>(data::loading, sizeof(data::loading));
		ctx->res.general.pattern = std::make_shared<texture>(data::pattern, sizeof(data::pattern));
		ctx->res.general.pattern_dark = std::make_shared<texture>(data::pattern_dark, sizeof(data::pattern_dark));
		ctx->res.general.pattern_pride = std::make_shared<texture>(data::pattern_pride, sizeof(data::pattern_pride));
		ctx->res.general.hearts = std::make_shared<texture>(data::hearts_tile, sizeof(data::hearts_tile));
		ctx->res.general.hearts_tile_size = {static_cast<float>(data::hearts_tile_w), static_cast<float>(data::hearts_tile_h)};

		// Default local profile. It is embedded in the DLL; cloud builds can still
		// replace it with their downloaded avatar in finalize().
		ctx->user.avatar = std::make_shared<texture>(data::avatar, sizeof(data::avatar));
		ctx->user.username = XOR("User");
		draw.textures[GUI_HASH("gui_user_avatar")] = ctx->user.avatar;

		// pride slider ramp #f785a4 -> #a3d9ed. the rgba texture source keeps this
		// pointer and uploads it as-is (BGRA in memory = 0xAARRGGBB), so it is static
		{
			static uint32_t ramp[256]{};
			for (int i{}; i < 256; ++i)
			{
				const auto t = static_cast<float>(i) / 255.f;
				const auto lerp = [t](int a, int b) { return static_cast<uint32_t>(a + (b - a) * t + .5f); };
				ramp[i] = 0xFF000000u | (lerp(247, 163) << 16) | (lerp(133, 217) << 8) | lerp(164, 237);
			}
			ctx->res.general.slider_gradient = std::make_shared<texture>(ramp, 256u, 1u, 256u * 4u);
		}

		// pop sound on control clicks / slider release. the embedded wav plays straight
		// from memory; SND_ASYNC returns immediately and a new click restarts the sound.
		// (mci aliases are bound to the thread that opened them, which is why the
		// earlier mp3 path stayed silent when clicks arrived on the input thread)
		// preload: open the wave device once and keep the pcm buffer prepared, so a click
		// only rewinds + resubmits it (PlaySound reopens the device every call -> lag)
		static HWAVEOUT pop_out{};
		static WAVEHDR pop_hdr{};
		{
			const auto *wav = data::pop_sound;
			const auto wav_size = sizeof(data::pop_sound);
			WAVEFORMATEX fmt{};
			const uint8_t *pcm{};
			uint32_t pcm_len{};
			for (size_t at = 12; at + 8 <= wav_size;)
			{
				uint32_t chunk_len{};
				memcpy(&chunk_len, wav + at + 4, 4);
				if (!memcmp(wav + at, "fmt ", 4))
					memcpy(&fmt, wav + at + 8, 16);
				else if (!memcmp(wav + at, "data", 4))
				{
					pcm = wav + at + 8;
					pcm_len = static_cast<uint32_t>(std::min<size_t>(chunk_len, wav_size - at - 8));
				}
				at += 8 + chunk_len + (chunk_len & 1);
			}

			// skip the near-silent lead-in the mp3 decoder leaves at the start
			if (pcm && fmt.wBitsPerSample == 16 && fmt.nBlockAlign)
			{
				uint32_t skip{};
				while (skip + fmt.nBlockAlign <= pcm_len)
				{
					int16_t sample{};
					memcpy(&sample, pcm + skip, 2);
					if (sample > 256 || sample < -256)
						break;
					skip += fmt.nBlockAlign;
				}
				if (skip < pcm_len)
				{
					pcm += skip;
					pcm_len -= skip;
				}
			}

			if (pcm && pcm_len && fmt.wFormatTag == WAVE_FORMAT_PCM &&
				waveOutOpen(&pop_out, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
			{
				pop_hdr.lpData = reinterpret_cast<LPSTR>(const_cast<uint8_t *>(pcm));
				pop_hdr.dwBufferLength = pcm_len;
				if (waveOutPrepareHeader(pop_out, &pop_hdr, sizeof(pop_hdr)) != MMSYSERR_NOERROR)
				{
					waveOutClose(pop_out);
					pop_out = {};
				}
			}
		}

		// waveOutReset can wait on the audio driver. Signal a dedicated worker so
		// sound playback never stalls the Windows input thread or control updates.
		static HANDLE pop_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (pop_event)
		{
			std::thread([]
			{
				for (;;)
				{
					WaitForSingleObject(pop_event, INFINITE);
					if (pop_out)
					{
						waveOutReset(pop_out);
						waveOutWrite(pop_out, &pop_hdr, sizeof(pop_hdr));
					}
					else
						PlaySoundA(reinterpret_cast<LPCSTR>(data::pop_sound), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
				}
			}).detach();
		}

		ctx->tick_sound_callback = []()
		{
			if (pop_event)
				SetEvent(pop_event);
			else
				PlaySoundA(reinterpret_cast<LPCSTR>(data::pop_sound), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
		};

		draw.textures[GUI_HASH("gui_logo_head")] = ctx->res.general.logo_head;
		draw.textures[GUI_HASH("gui_logo_stripes")] = ctx->res.general.logo_stripes;
		draw.textures[GUI_HASH("gui_cursor")] = ctx->res.general.cursor;
		draw.textures[GUI_HASH("gui_loading")] = ctx->res.general.loader;
		draw.textures[GUI_HASH("gui_pattern")] = ctx->res.general.pattern;
		draw.textures[GUI_HASH("gui_pattern_dark")] = ctx->res.general.pattern_dark;
		draw.textures[GUI_HASH("gui_pattern_pride")] = ctx->res.general.pattern_pride;
		draw.textures[GUI_HASH("gui_hearts")] = ctx->res.general.hearts;
		draw.textures[GUI_HASH("gui_slider_gradient")] = ctx->res.general.slider_gradient;

		// apply the saved menu theme (Pride is the default)
		colors.set_theme(cfg.misc.theme.get().first_set_bit().value_or((char)cfg_t::theme_pride));

		ctx->res.icons.up = std::make_shared<texture>(data::icon_up, sizeof(data::icon_up));
		ctx->res.icons.down = std::make_shared<texture>(data::icon_down, sizeof(data::icon_down));
		ctx->res.icons.notify_clear = std::make_shared<texture>(data::icon_clear, sizeof(data::icon_clear));
		ctx->res.icons.copy = std::make_shared<texture>(data::icon_copy, sizeof(data::icon_copy));
		ctx->res.icons.paste = std::make_shared<texture>(data::icon_paste, sizeof(data::icon_paste));
		ctx->res.icons.add = std::make_shared<texture>(data::icon_add, sizeof(data::icon_add));
		ctx->res.icons.search = std::make_shared<texture>(data::icon_search, sizeof(data::icon_search));
		ctx->res.icons.settings = std::make_shared<texture>(data::icon_settings, sizeof(data::icon_settings));
		ctx->res.icons.bug = std::make_shared<texture>(data::icon_bug, sizeof(data::icon_bug));

		draw.textures[GUI_HASH("gui_icon_up")] = ctx->res.icons.up;
		draw.textures[GUI_HASH("gui_icon_down")] = ctx->res.icons.down;
		draw.textures[GUI_HASH("gui_icon_clear")] = ctx->res.icons.notify_clear;
		draw.textures[GUI_HASH("gui_icon_copy")] = ctx->res.icons.copy;
		draw.textures[GUI_HASH("gui_icon_paste")] = ctx->res.icons.paste;
		draw.textures[GUI_HASH("gui_icon_add")] = ctx->res.icons.add;
		draw.textures[GUI_HASH("gui_icon_search")] = ctx->res.icons.search;
		draw.textures[GUI_HASH("gui_icon_settings")] = ctx->res.icons.settings;
		draw.textures[GUI_HASH("gui_icon_bug")] = ctx->res.icons.bug;

		ctx->res.icons.keys[VK_MENU] = std::make_shared<texture>(data::key_alt, sizeof(data::key_alt));
		ctx->res.icons.keys[VK_DOWN] = std::make_shared<texture>(data::key_down, sizeof(data::key_down));
		ctx->res.icons.keys[VK_LEFT] = std::make_shared<texture>(data::key_left, sizeof(data::key_left));
		ctx->res.icons.keys[VK_RIGHT] = std::make_shared<texture>(data::key_right, sizeof(data::key_right));
		ctx->res.icons.keys[VK_UP] = std::make_shared<texture>(data::key_up, sizeof(data::key_up));
		ctx->res.icons.keys[VK_BACK] = std::make_shared<texture>(data::key_back, sizeof(data::key_back));
		ctx->res.icons.keys[VK_CAPITAL] = std::make_shared<texture>(data::key_caps, sizeof(data::key_caps));
		ctx->res.icons.keys[VK_CONTROL] = std::make_shared<texture>(data::key_ctrl, sizeof(data::key_ctrl));
		ctx->res.icons.keys[VK_DELETE] = std::make_shared<texture>(data::key_delete, sizeof(data::key_delete));
		ctx->res.icons.keys[VK_END] = std::make_shared<texture>(data::key_end, sizeof(data::key_end));
		ctx->res.icons.keys[VK_HOME] = std::make_shared<texture>(data::key_home, sizeof(data::key_home));
		ctx->res.icons.keys[VK_INSERT] = std::make_shared<texture>(data::key_ins, sizeof(data::key_ins));
		ctx->res.icons.keys[VK_PRIOR] = std::make_shared<texture>(data::key_pgup, sizeof(data::key_pgup));
		ctx->res.icons.keys[VK_NEXT] = std::make_shared<texture>(data::key_pgdn, sizeof(data::key_pgdn));
		ctx->res.icons.keys[VK_RETURN] = std::make_shared<texture>(data::key_ret, sizeof(data::key_ret));
		ctx->res.icons.keys[VK_SHIFT] = std::make_shared<texture>(data::key_shift, sizeof(data::key_shift));
		ctx->res.icons.keys[VK_SPACE] = std::make_shared<texture>(data::key_space, sizeof(data::key_space));
		ctx->res.icons.keys[VK_TAB] = std::make_shared<texture>(data::key_tab, sizeof(data::key_tab));
		ctx->res.icons.keys[VK_LBUTTON] = std::make_shared<texture>(data::key_m1, sizeof(data::key_m1));
		ctx->res.icons.keys[VK_RBUTTON] = std::make_shared<texture>(data::key_m2, sizeof(data::key_m2));
		ctx->res.icons.keys[VK_MBUTTON] = std::make_shared<texture>(data::key_m3, sizeof(data::key_m3));
		ctx->res.icons.keys[VK_XBUTTON1] = std::make_shared<texture>(data::key_m4, sizeof(data::key_m4));
		ctx->res.icons.keys[VK_XBUTTON2] = std::make_shared<texture>(data::key_m5, sizeof(data::key_m5));
		ctx->res.icons.keys[VK_MULTIPLY] = std::make_shared<texture>(data::key_num_mul, sizeof(data::key_num_mul));
		ctx->res.icons.keys[VK_ADD] = std::make_shared<texture>(data::key_num_add, sizeof(data::key_num_add));
		ctx->res.icons.keys[VK_SEPARATOR] = std::make_shared<texture>(data::key_num_dec, sizeof(data::key_num_dec));
		ctx->res.icons.keys[VK_SUBTRACT] = std::make_shared<texture>(data::key_num_sub, sizeof(data::key_num_sub));
		ctx->res.icons.keys[VK_DECIMAL] = std::make_shared<texture>(data::key_num_dec, sizeof(data::key_num_dec));
		ctx->res.icons.keys[VK_DIVIDE] = std::make_shared<texture>(data::key_num_div, sizeof(data::key_num_div));
		ctx->res.icons.keys[VK_NUMPAD0] = std::make_shared<texture>(data::key_num0, sizeof(data::key_num0));
		ctx->res.icons.keys[VK_NUMPAD1] = std::make_shared<texture>(data::key_num1, sizeof(data::key_num1));
		ctx->res.icons.keys[VK_NUMPAD2] = std::make_shared<texture>(data::key_num2, sizeof(data::key_num2));
		ctx->res.icons.keys[VK_NUMPAD3] = std::make_shared<texture>(data::key_num3, sizeof(data::key_num3));
		ctx->res.icons.keys[VK_NUMPAD4] = std::make_shared<texture>(data::key_num4, sizeof(data::key_num4));
		ctx->res.icons.keys[VK_NUMPAD5] = std::make_shared<texture>(data::key_num5, sizeof(data::key_num5));
		ctx->res.icons.keys[VK_NUMPAD6] = std::make_shared<texture>(data::key_num6, sizeof(data::key_num6));
		ctx->res.icons.keys[VK_NUMPAD7] = std::make_shared<texture>(data::key_num7, sizeof(data::key_num7));
		ctx->res.icons.keys[VK_NUMPAD8] = std::make_shared<texture>(data::key_num8, sizeof(data::key_num8));
		ctx->res.icons.keys[VK_NUMPAD9] = std::make_shared<texture>(data::key_num9, sizeof(data::key_num9));

		for (const auto &[k, t] : ctx->res.icons.keys)
			draw.textures[GUI_HASH("gui_icon_key") + k] = t;

		// menu resources
		draw.textures[GUI_HASH("icon_rage")] = std::make_shared<texture>(data::icon_rage, sizeof(data::icon_rage));
		draw.textures[GUI_HASH("icon_legit")] = std::make_shared<texture>(data::icon_legit, sizeof(data::icon_legit));
		draw.textures[GUI_HASH("icon_visuals")] = std::make_shared<texture>(data::icon_visuals, sizeof(data::icon_visuals));
		draw.textures[GUI_HASH("icon_misc")] = std::make_shared<texture>(data::icon_misc, sizeof(data::icon_misc));
		draw.textures[GUI_HASH("icon_scripts")] = std::make_shared<texture>(data::icon_scripts, sizeof(data::icon_scripts));
		draw.textures[GUI_HASH("icon_skins")] = std::make_shared<texture>(data::icon_skins, sizeof(data::icon_skins));

		// hack resources
		draw.textures[GUI_HASH("icon_cloud")] = std::make_shared<texture>(data::icon_cloud, sizeof(data::icon_cloud));
		draw.textures[GUI_HASH("icon_file")] = std::make_shared<texture>(data::icon_file, sizeof(data::icon_file));
		draw.textures[GUI_HASH("icon_refresh")] = std::make_shared<texture>(data::icon_refresh, sizeof(data::icon_refresh));
		draw.textures[GUI_HASH("icon_save")] = std::make_shared<texture>(data::icon_save, sizeof(data::icon_save));
		draw.textures[GUI_HASH("icon_configs")] = std::make_shared<texture>(data::icon_configs, sizeof(data::icon_configs));
		draw.textures[GUI_HASH("icon_keys")] = std::make_shared<texture>(data::icon_keys, sizeof(data::icon_keys));
		draw.textures[GUI_HASH("icon_info")] = std::make_shared<texture>(data::icon_info, sizeof(data::icon_info));
		draw.textures[GUI_HASH("icon_close")] = std::make_shared<texture>(data::icon_close, sizeof(data::icon_close));
		draw.textures[GUI_HASH("icon_load")] = std::make_shared<texture>(data::icon_load, sizeof(data::icon_load));
		draw.textures[GUI_HASH("icon_import")] = std::make_shared<texture>(data::icon_import, sizeof(data::icon_import));
		draw.textures[GUI_HASH("icon_export")] = std::make_shared<texture>(data::icon_export, sizeof(data::icon_export));
		draw.textures[GUI_HASH("icon_delete")] = std::make_shared<texture>(data::icon_delete, sizeof(data::icon_delete));
		draw.textures[GUI_HASH("icon_autoload")] = std::make_shared<texture>(data::icon_autoload, sizeof(data::icon_autoload));
		draw.textures[GUI_HASH("icon_allow_insecure")] = std::make_shared<texture>(data::icon_allow_insecure, sizeof(data::icon_allow_insecure));
		draw.textures[GUI_HASH("icon_cloud_upd")] = std::make_shared<texture>(data::icon_cloud_upd, sizeof(data::icon_cloud_upd));

		ctx->add_chord(chord{std::vector<char>{VK_CONTROL}, std::vector<char>{'S'}, on_quicksave});

		ctx->add_chord(chord{std::vector<char>{VK_CONTROL}, std::vector<char>{'F'}, on_search});

#if defined(_DEBUG)
		ctx->add_chord(
			chord{std::vector<char>{VK_CONTROL, VK_SHIFT}, std::vector<char>{'I'}, []()
			{
				debug.enabled = !debug.enabled;
			}});

		ctx->add_chord(
			chord{std::vector<char>{VK_CONTROL, VK_SHIFT}, std::vector<char>{'R'}, []()
			{
				ctx->active = nullptr;
				for (const auto &c : ctx->active_popups)
					c->close();
			}});
#endif

		const auto wnd = make_window(
			XOR("wnd_main"), {50.f, 50.f}, {848.f, 588.f}, [](std::shared_ptr<layout> &e)
			{
				e->add(MAKE_TAB("rage", "RAGE")->make_selected());

				const auto legit = MAKE_TAB("legit", "LEGIT");
				legit->is_taking_input = false;
				legit->is_disabled = true;
				e->add(legit);

				e->add(MAKE_TAB("visuals", "VISUALS"));
				e->add(MAKE_TAB("misc", "MISC"));
				e->add(MAKE_TAB("skins", "SKINS"));

				const auto scripts = MAKE_TAB("scripts", "LUA");
				scripts->is_taking_input = false;
				scripts->is_disabled = true;
				e->add(scripts);
			}, [&](std::shared_ptr<layout> &e)
			{
				const auto empty_tab = [](const std::shared_ptr<layout> &) {};

				e->add(make_tab_area(GUI_HASH("rage_container"), true, s_none, group::rage_tab));
				e->add(make_tab_area(GUI_HASH("legit_container"), false, s_none, empty_tab));
				e->add(make_tab_area(GUI_HASH("visuals_container"), false, s_none, group::visuals_tab));
				e->add(make_tab_area(GUI_HASH("misc_container"), false, s_none, group::misc_tab));
				e->add(make_tab_area(GUI_HASH("skins_container"), false, s_none, group::skins_tab));
				e->add(make_tab_area(GUI_HASH("scripts_container"), false, s_none, empty_tab));
			}, [&](std::shared_ptr<layout> &e)
			{
				{
					const auto s = MAKE("tools>search", button, "", ctx->res.icons.search)->adjust_margin({0.f, 0.f, 8.f, 0.f})->adjust_size({20.f, 20.f})->as<button>();
					s->callback = []()
					{
						utils::debug::log(utils::debug::level::trace, "SEARCH", "toolbar search clicked");
						if (!ctx || !menu::men.is_open())
						{
							utils::debug::log(utils::debug::level::warning, "SEARCH", "toolbar search skipped: ctx=%p menu=%d", ctx.get(), menu::men.is_open());
							return;
						}

						const auto s = ctx->find(GUI_HASH("tools>search"));
						if (!s)
						{
							utils::debug::log(utils::debug::level::error, "SEARCH", "toolbar search control not found");
							return;
						}

						const auto p = std::make_shared<search_popup>(control_id{GUI_HASH("search")}, s->pos_abs() - vec2{0.f, 200.f});
						if (!safe_search_open(p.get()))
							utils::debug::log(utils::debug::level::error, "SEARCH", "toolbar search popup failed");
					};
					s->tooltip = XOR("Search");
					s->disable_id_display = true;
					e->add(s);
				}
				{
					const auto s = MAKE("tools>save", button, "", draw.textures[GUI_HASH("icon_save")])->adjust_margin({0.f, 0.f, 8.f, 0.f})->adjust_size({20.f, 20.f})->
																										 as<button>();
					s->tooltip = XOR("Quicksave");
					s->callback = on_quicksave;
					s->disable_id_display = true;
					e->add(s);
				}
				{
					const auto s = MAKE("tools>configs", button, "", draw.textures[GUI_HASH("icon_configs")])->adjust_margin({0.f, 0.f, 8.f, 0.f})->adjust_size(
						{20.f, 20.f})->as<button>();
					s->callback = []()
					{
						utils::debug::log(utils::debug::level::trace, "CFG", "toolbar configs clicked");
						if (!ctx || !menu::men.is_open())
						{
							utils::debug::log(utils::debug::level::warning, "CFG", "toolbar configs skipped: ctx=%p menu=%d", ctx.get(), menu::men.is_open());
							return;
						}

						const auto s = ctx->find(GUI_HASH("tools>configs"));
						if (!s)
						{
							utils::debug::log(utils::debug::level::error, "CFG", "toolbar configs control not found");
							return;
						}

						const auto p = std::make_shared<configs_popup>(control_id{GUI_HASH("configs_popup")}, s->pos_abs() - vec2{0.f, 240.f});
						if (!safe_configs_open(p.get()))
							utils::debug::log(utils::debug::level::error, "CFG", "toolbar configs popup failed");
					};
					s->tooltip = XOR("Configs");
					s->disable_id_display = true;
					e->add(s);
				}
				/* TODO: {
					const auto s = MAKE("tools>keys", button, "", draw.textures[GUI_HASH("icon_keys")])->adjust_margin({ 0.f, 0.f, 8.f, 0.f })->adjust_size({ 20.f, 20.f })->as<button>();
					s->callback = []()
					{
						static const auto s = ctx->find(GUI_HASH("tools>hotkeys"));
						const auto p = std::make_shared<hotkey_overview_popup>(control_id{ GUI_HASH("hotkey_overview_popup") });
						p->open();
					};
					s->tooltip = XOR("Hotkeys");
					e->add(s);
				}*/
				{
					const auto s = MAKE("tools>info", button, "", draw.textures[GUI_HASH("icon_info")])->adjust_margin({0.f, 0.f, 8.f, 0.f})->adjust_size({20.f, 20.f})->
																										 as<button>();
					s->callback = []()
					{
						const auto p = std::make_shared<gui::popups::about_popup>(control_id{GUI_HASH("about")});
						p->open();
					};
					s->disable_id_display = true;
					s->tooltip = XOR("About");
					e->add(s);
				}
			});

		ctx->add(wnd);

		main_wnd = wnd;
	}

	void menu_t::toggle()
	{
		const auto wnd = main_wnd.lock();
		if (!wnd)
			return;

		wnd->is_visible = !wnd->is_visible;
		wnd->is_taking_input = !wnd->is_taking_input;

		if (!wnd->is_visible)
		{
			ctx->active = nullptr;
			for (const auto &p : ctx->active_popups)
				p->close();
		}

		// input_system may not be captured on this build; toggling the menu must
		// not crash the render thread because of it.
		if (sdk::input_system)
			sdk::input_system->set_input(!wnd->is_visible);

		if (wnd->is_visible && draw.adapter)
		{
			const auto window_center = wnd->pos + wnd->size * draw.scale * 0.5f;
			POINT pt{static_cast<LONG>(window_center.x), static_cast<LONG>(window_center.y)};
			ClientToScreen(draw.adapter->window, &pt);
			//sdk::input_system->(pt.x, pt.y);
		}
	}

	bool menu_t::is_open()
	{
		const auto wnd = main_wnd.lock();
		return wnd && wnd->is_visible;
	}

	void menu_t::finalize()
	{
		if (finalized)
		{
			/*if (men.skins_ready)
				inventorychanger::update_selected(inventorychanger::force_update);*/
			return;
		}

		/*std::thread([]()
		{
			while (!men.skins_loaded)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				std::this_thread::yield();
			}

			std::lock_guard lock(menu::men.render_mtx);
			inventorychanger::fill_weapon_list();
			erase_fn(inventorychanger::fill_weapon_list);

			std::string s{};
			inventorychanger::fill_skin_list(s);
			inventorychanger::load();

			men.full_cycle_reset = true;
			men.skins_ready = true;
		}).detach();*/

#ifndef CS2_CLOUD
		ctx->user.username = XOR("User");
		ctx->user.active_until = XOR("never");
		ctx->user.is_alpha = false;
#else
		ctx->user.username = globals::user_info.name;
		ctx->user.active_until = globals::user_info.expiry;
		ctx->user.is_alpha = globals::user_info.alpha;

		auto &img = globals::user_info.img;
		if (!img.empty())
		{
			ctx->user.avatar = std::make_shared<texture>(img.data(), img.size());
			ctx->user.avatar->create();
			draw.textures[GUI_HASH("gui_user_avatar")] = ctx->user.avatar;
		}
#endif

		finalized = true;
	}

}
