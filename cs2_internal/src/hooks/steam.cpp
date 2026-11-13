#include <macros.h>
#include <game/draw_manager.h>
#include <game/game.h>
#include <game/hook_manager.h>
#include <gui/gui.h>
#include <menu/menu.h>
#include <menu/init/init_anim.h>
#include <ren/renderer.h>
#include <ren/adapters/adapter_dx11.h>
#include <resources/figtree_medium.h>
#include <resources/figtree_semibold.h>
#include <resources/smallest_pixel.h>
#include <sdk/globalvars.h>
#include <utils/debug_log.h>

namespace hooks::steam
{
	using namespace evo::gui;
	using namespace evo::ren;
	static ID3D11RenderTargetView* view{};
	static IDXGISwapChain* view_chain{};
	static ID3D11Texture2D* view_back_buffer{};
	static ID3D11Device* bound_device{};
	static ID3D11DeviceContext* bound_context{};
	static HWND bound_window{};

	static void release_render_target()
	{
		// the context holds its own references, so releasing only our pointers can
		// still leave the swap-chain buffers alive and make ResizeBuffers fail.
		if (bound_context)
		{
			ID3D11RenderTargetView* empty_view{};
			ID3D11ShaderResourceView* empty_resource{};
			bound_context->OMSetRenderTargets(1, &empty_view, nullptr);
			bound_context->PSSetShaderResources(0, 1, &empty_resource);
		}

		if (view)
			view->Release();
		if (view_back_buffer)
			view_back_buffer->Release();

		view = nullptr;
		view_chain = nullptr;
		view_back_buffer = nullptr;

		if (draw.adapter)
			std::static_pointer_cast<adapter_dx11>(draw.adapter)->release_back_buffer();
	}

	HRESULT resize_buffers(IDXGISwapChain* chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT format, UINT flags)
	{
		const auto original = hk_resize_buffers.GetOriginal();
		if (!original)
			return E_FAIL;

		release_render_target();
		return original(chain, buffer_count, width, height, format, flags);
	}

	bool __fastcall create_swap_chain(ISwapChainDx11* self, __int64* factory, __int64 device, char flags)
	{
		const auto original = hkCreateSwapChain.GetOriginal();
		if (!original)
			return false;

		const bool result = original(self, factory, device, flags);
		if (result && self && self->pDXGISwapChain)
		{
			sdk::swap_chain = self;
			if (!hkPresent.IsHooked())
				hkPresent.Create(utils::GetVFunc(sdk::swap_chain->pDXGISwapChain, 8U), reinterpret_cast<void*>(&present));
			if (!hk_resize_buffers.IsHooked())
				hk_resize_buffers.Create(utils::GetVFunc(sdk::swap_chain->pDXGISwapChain, 13U), reinterpret_cast<void*>(&resize_buffers));
		}

		return result;
	}

	HRESULT present(IDXGISwapChain *chain, UINT sync, UINT flags)
	{
		const auto oPresent = hkPresent.GetOriginal();
		if (!chain || !oPresent)
			return E_FAIL;

		ID3D11Device* dev{};
		ID3D11DeviceContext* device_context{};
		DXGI_SWAP_CHAIN_DESC chain_desc{};
		if (FAILED(chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&dev))) || !dev)
			return oPresent(chain, sync, flags);

		dev->GetImmediateContext(&device_context);
		if (!device_context || FAILED(chain->GetDesc(&chain_desc)))
		{
			if (device_context)
				device_context->Release();
			dev->Release();
			return oPresent(chain, sync, flags);
		}

		auto release_device = [&]
		{
			if (device_context)
				device_context->Release();
			if (dev)
				dev->Release();
		};

		static int present_frame = 0;
		++present_frame;

		// animations advance by the time since the previous present. measure it here
		// instead of trusting sdk::global_vars: that pointer comes from an unvalidated
		// client.dll offset (sdk::input already reads null on this build) and a wrong
		// frame_time freezes every animator - the intro and the menu then record draws
		// at alpha zero and nothing appears on screen at all.
		static LARGE_INTEGER qpc_freq{}, qpc_last{};
		static bool qpc_started = false;
		if (!qpc_freq.QuadPart)
			QueryPerformanceFrequency(&qpc_freq);

		LARGE_INTEGER qpc_now{};
		QueryPerformanceCounter(&qpc_now);

		float frame_time = 1.f / 60.f;
		if (qpc_started && qpc_freq.QuadPart)
			frame_time = static_cast<float>(qpc_now.QuadPart - qpc_last.QuadPart) / static_cast<float>(qpc_freq.QuadPart);

		qpc_last = qpc_now;
		qpc_started = true;

		if (frame_time < 0.0005f)
			frame_time = 0.0005f;
		else if (frame_time > 0.1f)
			frame_time = 0.1f;

		const auto is_first_adapter = !draw.adapter;
		const auto backend_changed = is_first_adapter || dev != bound_device || device_context != bound_context || chain_desc.OutputWindow != bound_window;
		const auto target_changed = view_chain != chain;
		if (backend_changed || target_changed)
			release_render_target();

		if (!view)
		{
			if (FAILED(chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&view_back_buffer))) || !view_back_buffer)
			{
				release_device();
				return oPresent(chain, sync, flags);
			}

			D3D11_TEXTURE2D_DESC bb{};
			view_back_buffer->GetDesc(&bb);

			// match the resource format and sample count so the recreated view is
			// valid for both single-sample and MSAA swap chains.
			D3D11_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = bb.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

			HRESULT rtv_result = dev->CreateRenderTargetView(view_back_buffer, &desc, &view);
			if (FAILED(rtv_result) || !view)
			{
				view = nullptr;
				rtv_result = dev->CreateRenderTargetView(view_back_buffer, nullptr, &view);
			}

			utils::debug::log(SUCCEEDED(rtv_result) && view ? utils::debug::level::ok : utils::debug::level::error,
				"D3D11", "render target view hr=0x%08lX view=%p back_buffer=%ux%u samples=%u format=%d",
				static_cast<unsigned long>(rtv_result), view, bb.Width, bb.Height, bb.SampleDesc.Count, static_cast<int>(bb.Format));

			if (FAILED(rtv_result) || !view)
			{
				release_render_target();
				release_device();
				return oPresent(chain, sync, flags);
			}

			view_chain = chain;
		}

		if (backend_changed)
		{
			utils::debug::log(utils::debug::level::info, "D3D11", "binding adapter window=%p %ux%u samples=%u",
				chain_desc.OutputWindow, chain_desc.BufferDesc.Width, chain_desc.BufferDesc.Height, chain_desc.SampleDesc.Count);

			draw.set_adapter(std::make_shared<adapter_dx11>(chain_desc.OutputWindow, dev, device_context));
			if (is_first_adapter)
			{
				draw.create_layer(game_layer);
				draw.create_layer(17);
				draw.create_layer(63);
				menu::init_anim.on_adapter_ready();
			}

			// layers keep their identity, but their GPU buffers and deferred command
			// lists belong to the new device now.
			draw_mgr.buf = draw.layers[game_layer];
			draw_mgr.buf->is_deferred = true;
			draw_mgr.buf->skip_dpi = true;
			draw_mgr.dirty_ackn = false;

			if (bound_context)
				bound_context->Release();
			if (bound_device)
				bound_device->Release();

			bound_device = dev;
			bound_context = device_context;
			bound_window = chain_desc.OutputWindow;
			dev = nullptr;
			device_context = nullptr;

			draw.refresh();
			draw.create_objects();
		}

		// watchdog: a stalled intro chain must never leave the screen empty - after
		// ~15s of intro frames just build the menu.
		static int intro_frames = 0;
		if (!menu::init_anim.init_done && ++intro_frames == 600)
		{
			utils::debug::log(utils::debug::level::warning, "MENU", "intro watchdog fired after %d frames, forcing menu build", intro_frames);
			menu::init_anim.init_ready = true;
		}

		static bool logged_intro_done = false;
		if (!logged_intro_done && menu::init_anim.init_ready)
		{
			logged_intro_done = true;
			utils::debug::log(utils::debug::level::ok, "MENU", "intro animation finished, building menu");
		}

		if (menu::init_anim.init_ready && !menu::init_anim.init_done)
		{
			menu::men.update_dpi_scale();
			ctx = std::make_unique<context>();

			// esp fonts come from the recovered font set, embedded - no GDI/OS font dependency
			draw.fonts[FNV1A("esp")] = std::make_shared<font>(
				smallest_pixel.data(), smallest_pixel.size(), 11.f, font_flag_outline | font_flag_no_dpi, 0, 0x45F);
			draw.fonts[FNV1A("esp_name")] = std::make_shared<font>(
				figtree_semibold.data(), figtree_semibold.size(), 13.f, font_flag_shadow | font_flag_no_dpi, 0, 0x45F);
			draw.fonts[FNV1A("lby")] = std::make_shared<font>(
				figtree_semibold.data(), figtree_semibold.size(), 27.f, font_flag_shadow, 0, 0x80);
			menu::men.init();

			draw.refresh();
			draw.create_objects();

			utils::debug::log(utils::debug::level::ok, "MENU", "menu initialized");
			menu::init_anim.init_done = true;
		}

		static_cast<ID3D11DeviceContext *>(draw.adapter->get_context())->OMSetRenderTargets(1, &view, nullptr);

		draw.frame_time = frame_time;
		draw.time += frame_time;

		draw.refresh();

		// the viewport, projection and scissor all derive from draw.display, so it has
		// to match the back buffer we render into. GetClientRect can read 0x0 and then
		// every draw is scissored away - trust the swap chain size instead.
		{
			DXGI_SWAP_CHAIN_DESC sd{};
			if (SUCCEEDED(chain->GetDesc(&sd)) && sd.BufferDesc.Width > 1 && sd.BufferDesc.Height > 1)
			{
				draw.display = vec2{static_cast<float>(sd.BufferDesc.Width), static_cast<float>(sd.BufferDesc.Height)};
				draw.DisplaySize = draw.display;
			}
		}

		// Automatic DPI follows the active swap-chain resolution, not just the
		// resolution that happened to be active when the menu was initialized.
		static vec2 last_display{};
		if (last_display != draw.display)
		{
			last_display = draw.display;
			if (menu::init_anim.init_done)
				menu::men.update_dpi_scale();
		}

		if (menu::init_anim.init_done)
		{
			if (menu::men.full_cycle_reset)
			{
				menu::men.full_cycle_reset = false;
				menu::group::rebuild_legit_layout();
				menu::group::rebuild_rage_layout();
				ctx->reset();
				ctx->should_update_old_value = false;
			}

			std::lock_guard lock(menu::men.render_mtx);
			draw.begin_frame();
			ctx->render();

			menu::men.finalize();
		}
		else
		{
			draw.begin_frame();
			menu::init_anim.render();
		}

		draw.end_frame();

		//handle synced texture resets
		if (draw.layers[game_layer]->deferred_dirty && draw_mgr.dirty_ackn)
		{
			draw.layers[game_layer]->cb->reset(true);
			draw.layers[game_layer]->deferred_dirty = false;
			draw_mgr.dirty_ackn = false;
		}

		draw.render();

		// frame trace: proves presents keep flowing and shows why nothing draws
		// (zero layer sizes = nothing recorded; zero display = all scissored away).
		if (present_frame <= 8 || present_frame % 120 == 0)
		{
			const auto l63 = draw.layers[63] ? draw.layers[63]->cb->real_size : size_t{};
			const auto l32 = draw.layers[32] ? draw.layers[32]->cb->real_size : size_t{};
			const float gv_ft = sdk::global_vars ? sdk::global_vars->frame_time : -1.f;
			utils::debug::log(utils::debug::level::info, "D3D11",
				"frame #%d display=%.0fx%.0f ft=%.4f gv_ft=%.4f l63=%llu l32=%llu intro ready=%d done=%d",
				present_frame, draw.display.x, draw.display.y, frame_time, gv_ft,
				static_cast<unsigned long long>(l63), static_cast<unsigned long long>(l32),
				menu::init_anim.init_ready ? 1 : 0, menu::init_anim.init_done ? 1 : 0);
		}

		draw.flush();

		// handle safe font reset
		if (menu::men.font_reset)
		{
			draw.layers[game_layer]->deferred_dirty = true;
			// update fonts
			for (auto &[_, f] : draw.fonts)
			{
				if (!f)
					continue;

				f->destroy();
				f->create();
			}

			menu::men.font_was_reset = true;
			menu::men.font_reset = false;
		}

		release_device();
		return oPresent(chain, sync, flags);
	}
} // namespace hooks::steam
