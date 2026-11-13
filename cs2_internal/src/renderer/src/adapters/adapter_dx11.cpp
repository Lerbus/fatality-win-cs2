#include <ren/adapters/adapter_dx11.h>
#include <ren/buffers/ib_dx11.h>
#include <ren/buffers/vb_dx11.h>
#include <ren/renderer.h>
#include <utils/debug_log.h>

namespace evo::ren
{
	void adapter_dx11::on_create_objects()
	{
		if (frag)
			return;

		REN_DBG("dx11: creating objects");

		// create deferred context
		if (FAILED(device->CreateDeferredContext(0, &deferred)))
			throw;

		vert = (ID3D11VertexShader *)create_vertex_shader("cbuffer vertex_buffer:register(b0){float4x4 projection_matrix;};struct VS_INPUT{float2 pos:POSITION;float4 "
														  "col:COLOR0;float2 uv:TEXCOORD0;};struct PS_INPUT{float4 pos:SV_POSITION;float4 col:COLOR0;float2 "
														  "uv:TEXCOORD0;};PS_INPUT main(VS_INPUT input){PS_INPUT "
														  "output;output.pos=mul(projection_matrix,float4(input.pos.xy,0.f,1.f));output.col=input.col;output.uv=input.uv;"
														  "return output;}");
		frag = (ID3D11PixelShader *)create_fragment_shader("struct PS_INPUT{float4 pos:SV_POSITION;float4 col:COLOR0;float2 uv:TEXCOORD0;};sampler sampler0;Texture2D "
														   "texture0;float4 main(PS_INPUT input):SV_Target{return input.col*texture0.Sample(sampler0,input.uv);}");

		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = sizeof(float[4][4]);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		if (FAILED(device->CreateBuffer(&desc, nullptr, &constant_buffer)))
			throw;

		// pixel-stage constants for custom fragment shaders (blur taps, time, alpha)
		desc.ByteWidth = sizeof(pixel_constant_buffer);
		if (FAILED(device->CreateBuffer(&desc, nullptr, &constant_buffer_ps)))
			throw;

		D3D11_BLEND_DESC blend{};
		blend.AlphaToCoverageEnable = false;
		blend.RenderTarget[0].BlendEnable = true;
		blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(device->CreateBlendState(&blend, &blend_state)))
			throw;

		D3D11_RASTERIZER_DESC rast{};
		rast.FillMode = D3D11_FILL_SOLID;
		rast.CullMode = D3D11_CULL_NONE;
		rast.ScissorEnable = true;
		rast.DepthClipEnable = true;

		if (FAILED(device->CreateRasterizerState(&rast, &rasterizer_state)))
			throw;

		D3D11_DEPTH_STENCIL_DESC depth{};
		depth.DepthEnable = false;
		depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depth.StencilEnable = false;
		depth.FrontFace.StencilFailOp = depth.FrontFace.StencilDepthFailOp = depth.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth.BackFace = depth.FrontFace;

		if (FAILED(device->CreateDepthStencilState(&depth, &depth_stencil)))
			throw;

		D3D11_SAMPLER_DESC samp{};
		samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samp.MipLODBias = 0.f;
		samp.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samp.MinLOD = 0.f;
		samp.MaxLOD = 0.f;

		if (FAILED(device->CreateSamplerState(&samp, &texture_sampler)))
			throw;

		// point sampler for the font atlas: glyphs are pixel-snapped, so 1:1
		// point sampling keeps them crisp instead of linearly softened
		samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samp.AddressU = samp.AddressV = samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		if (FAILED(device->CreateSamplerState(&samp, &point_sampler)))
			throw;

		constexpr auto data = 0xFFFFFFFF;
		texture_object = static_cast<ID3D11ShaderResourceView *>(create_texture((char *)&data, 1, 1, 4));

		// a null shader draws nothing at all - log what actually got created so a
		// silent compile failure cannot look like "nothing appeared".
		utils::debug::log(vert && frag ? utils::debug::level::ok : utils::debug::level::error,
			"D3D11", "objects created vert=%p frag=%p layout=%p cbuf=%p tex=%p",
			vert, frag, input_layout, constant_buffer, texture_object);

		probe_texture_orientation();
	}

	void adapter_dx11::probe_texture_orientation()
	{
		vtx_flip_v = false;
		if (!vert || !frag || !input_layout || !constant_buffer)
			return;

		// 1x2 source: row 0 (top) white, row 1 (bottom) black
		uint32_t px[2] = {0xFFFFFFFF, 0xFF000000};
		const auto src = static_cast<ID3D11ShaderResourceView *>(create_texture((char *)px, 1, 2, 4));

		D3D11_TEXTURE2D_DESC td{};
		td.Width = 1;
		td.Height = 2;
		td.MipLevels = td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET;

		ID3D11Texture2D *rt{}, *staging{};
		ID3D11RenderTargetView *rtv{};
		ID3D11Buffer *vbuf{};
		const auto cleanup = [&]
		{
			if (vbuf) vbuf->Release();
			if (rtv) rtv->Release();
			if (staging) staging->Release();
			if (rt) rt->Release();
			if (src) src->Release();
		};

		if (FAILED(device->CreateTexture2D(&td, nullptr, &rt)) || FAILED(device->CreateRenderTargetView(rt, nullptr, &rtv)))
			return cleanup();

		td.Usage = D3D11_USAGE_STAGING;
		td.BindFlags = 0;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		if (FAILED(device->CreateTexture2D(&td, nullptr, &staging)))
			return cleanup();

		// same quad layout add_rect_filled emits: v = 0 at the top edge
		const vertex quad[6] = {
			{0.f, 0.f, 0.f, 0xFFFFFFFF, 0.f, 0.f, true}, {1.f, 0.f, 0.f, 0xFFFFFFFF, 1.f, 0.f, true}, {1.f, 2.f, 0.f, 0xFFFFFFFF, 1.f, 1.f, true},
			{0.f, 0.f, 0.f, 0xFFFFFFFF, 0.f, 0.f, true}, {1.f, 2.f, 0.f, 0xFFFFFFFF, 1.f, 1.f, true}, {0.f, 2.f, 0.f, 0xFFFFFFFF, 0.f, 1.f, true},
		};
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(quad);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vd{quad};
		if (FAILED(device->CreateBuffer(&bd, &vd, &vbuf)))
			return cleanup();

		// save the bindings we overwrite
		ID3D11RenderTargetView *prev_rtv{};
		ID3D11DepthStencilView *prev_dsv{};
		ctx->OMGetRenderTargets(1, &prev_rtv, &prev_dsv);
		UINT nvp = 1, nsc = 1;
		D3D11_VIEWPORT prev_vp{};
		D3D11_RECT prev_sc{};
		ctx->RSGetViewports(&nvp, &prev_vp);
		ctx->RSGetScissorRects(&nsc, &prev_sc);

		// projection identical to render(), for a 1x2 display
		D3D11_MAPPED_SUBRESOURCE m{};
		if (SUCCEEDED(ctx->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m)))
		{
			const float mvp[4][4] = {
				{2.f / 1.f, 0.f, 0.f, 0.f},
				{0.f, 2.f / -2.f, 0.f, 0.f},
				{0.f, 0.f, 0.5f, 0.f},
				{-1.f, 1.f, 0.5f, 1.f},
			};
			memcpy(m.pData, mvp, sizeof(mvp));
			ctx->Unmap(constant_buffer, 0);
		}

		const D3D11_VIEWPORT vp{0.f, 0.f, 1.f, 2.f, 0.f, 1.f};
		const D3D11_RECT sc{0, 0, 1, 2};
		const float clear[4]{0.5f, 0.5f, 0.5f, 1.f};
		constexpr UINT stride = sizeof(vertex), offset = 0;
		float bs[4]{};

		ctx->OMSetRenderTargets(1, &rtv, nullptr);
		ctx->ClearRenderTargetView(rtv, clear);
		ctx->RSSetViewports(1, &vp);
		ctx->RSSetScissorRects(1, &sc);
		ctx->RSSetState(rasterizer_state);
		ctx->OMSetBlendState(blend_state, bs, 0xFFFFFFFF);
		ctx->OMSetDepthStencilState(depth_stencil, 0);
		ctx->IASetInputLayout(input_layout);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);
		ctx->VSSetShader(vert, nullptr, 0);
		ctx->VSSetConstantBuffers(0, 1, &constant_buffer);
		ctx->PSSetShader(frag, nullptr, 0);
		ctx->PSSetShaderResources(0, 1, &src);
		ctx->PSSetSamplers(0, 1, point_sampler ? &point_sampler : &texture_sampler);
		ctx->Draw(6, 0);

		ctx->CopyResource(staging, rt);

		// restore
		ctx->OMSetRenderTargets(1, &prev_rtv, prev_dsv);
		if (nvp) ctx->RSSetViewports(1, &prev_vp);
		if (nsc) ctx->RSSetScissorRects(1, &prev_sc);
		if (prev_rtv) prev_rtv->Release();
		if (prev_dsv) prev_dsv->Release();

		if (SUCCEEDED(ctx->Map(staging, 0, D3D11_MAP_READ, 0, &m)))
		{
			const auto top = *static_cast<const uint32_t *>(m.pData);
			const auto bottom = *reinterpret_cast<const uint32_t *>(static_cast<const char *>(m.pData) + m.RowPitch);
			ctx->Unmap(staging, 0);

			// only act on an unambiguous result: top dark and bottom bright
			vtx_flip_v = (top & 0xFF) < 0x40 && (bottom & 0xFF) > 0xC0;
			utils::debug::log(vtx_flip_v ? utils::debug::level::warning : utils::debug::level::ok, "D3D11",
				"texture orientation probe top=%08X bottom=%08X flip_v=%d", top, bottom, vtx_flip_v);
		}

		cleanup();
	}

	void adapter_dx11::on_destroy_objects()
	{
		if (!frag)
			return;

		REN_DBG("dx11: destroying objects");

		if (deferred)
			deferred->Release();
		if (deferred_list)
			deferred_list->Release();
		if (vert)
			vert->Release();
		if (frag)
			frag->Release();
		if (input_layout)
			input_layout->Release();
		if (depth_stencil)
			depth_stencil->Release();
		if (rasterizer_state)
			rasterizer_state->Release();
		if (blend_state)
			blend_state->Release();
		if (constant_buffer)
			constant_buffer->Release();
		if (constant_buffer_ps)
			constant_buffer_ps->Release();
		if (texture_sampler)
			texture_sampler->Release();
		if (point_sampler)
		{
			point_sampler->Release();
			point_sampler = nullptr;
		}
		if (texture_object)
			texture_object->Release();
		release_back_buffer();

		deferred_list = {};
		input_layout = {};
		frag = {};
	}

	void adapter_dx11::render()
	{
		D3D11_VIEWPORT vp{};
		vp.Width = draw.display.x;
		vp.Height = draw.display.y;
		vp.MinDepth = 0.f;
		vp.MaxDepth = 1.f;
		vp.TopLeftX = vp.TopLeftY = 0.f;

		D3D11_MAPPED_SUBRESOURCE cb{};
		if (FAILED(ctx->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb)))
			throw;

		const float mvp[4][4] = {
			{2.f / draw.display.x, 0.f, 0.f, 0.f},
			{0.f, 2.f / -draw.display.y, 0.f, 0.f},
			{0.f, 0.f, 0.5f, 0.f},
			{-1.f, 1.f, 0.5f, 1.f},
		};

		memcpy(&reinterpret_cast<vertex_constant_buffer *>(cb.pData)->mvp, &mvp, sizeof(float) * 16);
		ctx->Unmap(constant_buffer, 0);

		float bs_flt[4]{};
		ctx->IASetInputLayout(input_layout);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetConstantBuffers(0, 1, &constant_buffer);
		ctx->PSSetSamplers(0, 1, &texture_sampler);
		ctx->OMSetDepthStencilState(depth_stencil, 0);
		ctx->OMSetBlendState(blend_state, bs_flt, 0xFFFFFFFF);
		ctx->RSSetState(rasterizer_state);
		ctx->RSSetViewports(1, &vp);

		// capture the game frame once per frame, before any of our own layers are
		// drawn into the back buffer - the blur passes sample this capture
		update_back_buffer();

		for (const auto &l : draw.layers)
		{
			if (!l)
				continue;

			const bool deferred_dirty = l->deferred_dirty;
			const auto deferred_ready = l->is_deferred && l->is_deferred_ready && !deferred_dirty;

			if (deferred_ready)
				l->cb->swap(l->cb_deferred);

			if (deferred_dirty)
				l->cb_deferred->reset(true);

			const auto &cmds = l->is_deferred ? l->cb_deferred : l->cb;
			if (!cmds->real_size)
			{
				if (l->is_deferred)
					l->is_deferred_ready = false;

				continue;
			}

			// handle deferred rendering
			if (deferred_ready)
			{
				if (deferred_list)
					ctx->ExecuteCommandList(deferred_list, true);

				l->is_deferred_ready = false;
			}

			constexpr auto stride = static_cast<uint32_t>(sizeof(vertex));
			constexpr auto offset = 0u;

			ctx->IASetVertexBuffers(0, 1, reinterpret_cast<ID3D11Buffer **>(&l->vb->object), &stride, &offset);
			ctx->IASetIndexBuffer(static_cast<ID3D11Buffer *>(l->ib->object), DXGI_FORMAT_R16_UINT, 0);

			int vo{}, io{};
			cmds->reset();

			while (cmds->has_more_data())
			{
				const auto &cmd = cmds->read<command>();

				// blur passes bind the scene capture; the recorded pointer can be null on
				// the very first frame (the capture does not exist yet at record time)
				const auto tex = reinterpret_cast<ID3D11ShaderResourceView *>(
					cmd.texture ? cmd.texture : (cmd.frag_shader ? reinterpret_cast<void *>(back_buffer_ds) : reinterpret_cast<void *>(texture_object)));

				// with no capture a blur pass would sample black over the whole screen,
				// so drop it - the buffers still advance to keep later draws aligned
				if (!tex)
				{
					io += static_cast<int>(cmd.ic);
					vo += static_cast<int>(cmd.vc);
					continue;
				}

				ctx->PSSetShaderResources(0, 1, &tex);
				// text uses point sampling; everything else (blur, icons) stays linear
				ctx->PSSetSamplers(0, 1, cmd.point_filter && point_sampler ? &point_sampler : &texture_sampler);
				ctx->VSSetShader(cmd.vert_shader ? static_cast<ID3D11VertexShader *>(cmd.vert_shader) : vert, nullptr, 0);
				ctx->PSSetShader(cmd.frag_shader ? static_cast<ID3D11PixelShader *>(cmd.frag_shader) : frag, nullptr, 0);

				if (cmd.frag_shader)
					push_pixel_constants(cmd.alpha);

				// construct clip rect
				RECT clip;
				if (cmd.clip_rect)
				{
					const auto scale = cmd.ignore_scaling ? 1.f : draw.scale;
					clip = {
						static_cast<LONG>(cmd.clip_rect->mins.x * scale), static_cast<LONG>(cmd.clip_rect->mins.y * scale),
						static_cast<LONG>(cmd.clip_rect->maxs.x * scale), static_cast<LONG>(cmd.clip_rect->maxs.y * scale)};
				}
				else
					clip = {0, 0, static_cast<LONG>(draw.display.x), static_cast<LONG>(draw.display.y)};

				ctx->RSSetScissorRects(1, &clip);
				ctx->DrawIndexed(cmd.ic, io, vo);

				io += static_cast<int>(cmd.ic);
				vo += static_cast<int>(cmd.vc);
			}
		}
	}

	std::shared_ptr<buffer_base> adapter_dx11::alloc_ib()
	{
		REN_DBG("dx11: allocating index buffer");

		const auto ib = std::make_shared<ib_dx11_buffer>();
		ib->adapter = shared_from_this();
		return ib;
	}

	std::shared_ptr<buffer_base> adapter_dx11::alloc_vb()
	{
		REN_DBG("dx11: allocating vertex buffer");

		const auto vb = std::make_shared<vb_dx11_buffer>();
		vb->adapter = shared_from_this();
		return vb;
	}

	void *adapter_dx11::create_vertex_buffer(size_t sz)
	{
		REN_DBG_FMT("dx11: creating vertex buffer (size %zu)", sz);

		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sz;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Buffer *buf;
		if (FAILED(device->CreateBuffer(&desc, nullptr, &buf)))
			throw;

		return buf;
	}

	void adapter_dx11::destroy_vertex_buffer(void *vb)
	{
		REN_DBG_FMT("dx11: destroying vertex buffer (mem %p)", vb);
		((ID3D11Buffer *)vb)->Release();
	}

	void *adapter_dx11::create_index_buffer(size_t sz)
	{
		REN_DBG_FMT("dx11: creating index buffer (size %zu)", sz);

		D3D11_BUFFER_DESC desc{};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sz;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Buffer *buf;
		if (FAILED(device->CreateBuffer(&desc, nullptr, &buf)))
			throw;

		return buf;
	}

	void adapter_dx11::destroy_index_buffer(void *ib)
	{
		REN_DBG_FMT("dx11: destroying index buffer (mem %p)", ib);
		((ID3D11Buffer *)ib)->Release();
	}

	void *adapter_dx11::create_fragment_shader(const char *src)
	{
		REN_DBG("dx11: creating fragment shader");

		ID3D10Blob *blob{}, *errors{};
		const HRESULT compiled = D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &blob, &errors);
		if (FAILED(compiled) || !blob)
		{
			utils::debug::log(utils::debug::level::error, "D3D11", "ps compile failed hr=0x%08lX: %.300s",
				static_cast<unsigned long>(compiled), errors ? (const char *)errors->GetBufferPointer() : "no error blob");

			if (errors)
			{
				if (on_fragment_shader_compilation_error)
					on_fragment_shader_compilation_error((const char *)errors->GetBufferPointer());

				errors->Release();
			}

			return nullptr;
		}

		ID3D11PixelShader *shader{};
		if (FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader)))
		{
			utils::debug::log(utils::debug::level::error, "D3D11", "CreatePixelShader failed");
			blob->Release();
			return nullptr;
		}

		blob->Release();
		return shader;
	}

	void adapter_dx11::destroy_fragment_shader(void *fs)
	{
		REN_DBG_FMT("dx11: destroying fragment shader (mem %p)", fs);
		((ID3D11PixelShader *)fs)->Release();
	}

	void *adapter_dx11::create_vertex_shader(const char *src)
	{
		REN_DBG("dx11: creating vertex shader");

		ID3D10Blob *blob{}, *errors{};
		if (FAILED(D3DCompile(src, strlen(src), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &blob, &errors)))
		{
			if (errors)
			{
				if (on_vertex_shader_compilation_error)
					on_vertex_shader_compilation_error((const char *)errors->GetBufferPointer());

				errors->Release();
				return nullptr;
			}
		}

		if (!blob)
			return nullptr;

		ID3D11VertexShader *shader{};
		if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader)))
		{
			blob->Release();
			return nullptr;
		}

		static const D3D11_INPUT_ELEMENT_DESC layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		if (!input_layout)
		{
			if (FAILED(device->CreateInputLayout(layout, 3, blob->GetBufferPointer(), blob->GetBufferSize(), &input_layout)))
				throw;
		}

		blob->Release();
		return shader;
	}

	void adapter_dx11::destroy_vertex_shader(void *vs)
	{
		REN_DBG_FMT("dx11: destroying vertex shader (mem %p)", vs);
		((ID3D11VertexShader *)vs)->Release();
	}

	void *adapter_dx11::create_texture(char *data, uint32_t w, uint32_t h, uint32_t p)
	{
		REN_DBG_FMT("dx11: creating texture (mem %p, w %u, h %u, p %u)", data, w, h, p);

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = data;
		sd.SysMemPitch = p;
		sd.SysMemSlicePitch = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC r{};
		r.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		r.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		r.Texture2D.MipLevels = 1;
		r.Texture2D.MostDetailedMip = 0;

		ID3D11Texture2D *texture{};
		if (FAILED(device->CreateTexture2D(&desc, &sd, &texture)))
			throw;

		ID3D11ShaderResourceView *_tex{};
		if (FAILED(device->CreateShaderResourceView(texture, &r, &_tex)))
			throw;

		return _tex;
	}

	void adapter_dx11::update_texture(void *tex, char *data, uint32_t w, uint32_t h, uint32_t p)
	{
		REN_DBG_FMT("dx11: updating texture (tex %p, mem %p, w %u, h %u, p %u)", tex, data, w, h, p);

		const auto view = (ID3D11ShaderResourceView *)tex;

		ID3D11Texture2D *texture{};
		view->GetResource((ID3D11Resource **)&texture);

		D3D11_MAPPED_SUBRESOURCE rect{};
		ctx->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &rect);

		if (rect.RowPitch == p)
			memcpy(rect.pData, data, p * h);
		else
		{
			for (uint32_t i = 0; i < h; i++)
				memcpy((char *)rect.pData + rect.RowPitch * i, data + p * i, p);
		}

		ctx->Unmap(texture, 0);
	}

	void adapter_dx11::destroy_texture(void *tex)
	{
		REN_DBG_FMT("dx11: destroying texture (mem %p)", tex);
		((ID3D11ShaderResourceView *)tex)->Release();
	}

	void *adapter_dx11::get_back_buffer() { return back_buffer; }

	void *adapter_dx11::get_back_buffer_downsampled() { return back_buffer_ds; }

	void adapter_dx11::release_back_buffer()
	{
		if (back_buffer)
			back_buffer->Release();
		if (back_buffer_ds)
			back_buffer_ds->Release();
		if (back_buffer_ds_full)
			back_buffer_ds_full->Release();
		if (back_buffer_tex)
			back_buffer_tex->Release();
		if (back_buffer_ds_tex)
			back_buffer_ds_tex->Release();

		back_buffer = nullptr;
		back_buffer_ds = nullptr;
		back_buffer_ds_full = nullptr;
		back_buffer_tex = nullptr;
		back_buffer_ds_tex = nullptr;
	}

	void adapter_dx11::update_back_buffer(bool only_downsampled)
	{
		// capture whatever the game (and earlier layers) have drawn into the
		// current render target and keep a quarter-size copy of it around. the blur
		// shaders sample that copy: the downsampled view is mip 2 of the copy, so
		// uv 0..1 covers exactly the whole frame at 1/4 resolution.
		if (!back_buffer_tex)
		{
			ID3D11RenderTargetView *rtv{};
			ctx->OMGetRenderTargets(1, &rtv, nullptr);
			if (!rtv)
				return;

			ID3D11Resource *res{};
			rtv->GetResource(&res);
			rtv->Release();
			if (!res)
				return;

			res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&back_buffer_tex));
			res->Release();
			if (!back_buffer_tex)
				return;

			D3D11_TEXTURE2D_DESC d{};
			back_buffer_tex->GetDesc(&d);

			D3D11_TEXTURE2D_DESC c{};
			c.Width = d.Width;
			c.Height = d.Height;
			c.MipLevels = 0; // full chain, generated after the copy
			c.ArraySize = 1;
			// a typeless swap-chain format cannot be used for an SRV. the capture
			// texture and its views use the typed equivalent instead.
			auto capture_format = d.Format;
			switch (d.Format)
			{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS: capture_format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS: capture_format = DXGI_FORMAT_B8G8R8A8_UNORM; break;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS: capture_format = DXGI_FORMAT_R10G10B10A2_UNORM; break;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS: capture_format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
			default: break;
			}
			c.Format = capture_format;
			c.SampleDesc.Count = 1;
			c.Usage = D3D11_USAGE_DEFAULT;
			c.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			c.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

			if (FAILED(device->CreateTexture2D(&c, nullptr, &back_buffer_ds_tex)) || !back_buffer_ds_tex)
			{
				back_buffer_tex->Release();
				back_buffer_tex = {};
				return;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC r{};
			r.Format = c.Format;
			r.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			r.Texture2D.MostDetailedMip = 0;
			r.Texture2D.MipLevels = -1;
			if (FAILED(device->CreateShaderResourceView(back_buffer_ds_tex, &r, &back_buffer_ds_full)))
			{
				// do not keep a half-valid capture around: sampling it would produce
				// the fullscreen black rectangle seen before.
				back_buffer_ds_tex->Release();
				back_buffer_tex->Release();
				back_buffer_ds_tex = back_buffer_tex = nullptr;
				return;
			}

			// mip 2 = 1/4 of the frame, which is what the blur UV math expects.
			// if that view cannot be created, the full view is still a valid capture.
			r.Texture2D.MostDetailedMip = 2;
			r.Texture2D.MipLevels = 1;
			if (FAILED(device->CreateShaderResourceView(back_buffer_ds_tex, &r, &back_buffer_ds)) && back_buffer_ds_full)
			{
				back_buffer_ds = back_buffer_ds_full;
				back_buffer_ds_full->AddRef();
			}
		}

		if (!back_buffer_ds_tex || !back_buffer_ds_full || !back_buffer_ds)
			return;

		D3D11_TEXTURE2D_DESC d{};
		back_buffer_tex->GetDesc(&d);

		// the source is still bound as render target and the destination is still
		// bound as a shader resource from the previous blur pass - copying with
		// either one bound is a hazard and yields a black capture. unbind both,
		// copy, then restore the previous bindings.
		ID3D11RenderTargetView *prev_rtv{};
		ID3D11DepthStencilView *prev_dsv{};
		ID3D11ShaderResourceView *prev_srv{};
		ctx->OMGetRenderTargets(1, &prev_rtv, &prev_dsv);
		ctx->PSGetShaderResources(0, 1, &prev_srv);

		ID3D11RenderTargetView *null_rtv{};
		ID3D11ShaderResourceView *null_srv{};
		ctx->OMSetRenderTargets(1, &null_rtv, nullptr);
		ctx->PSSetShaderResources(0, 1, &null_srv);

		// msaa back buffers need a resolve before they can be copied
		if (d.SampleDesc.Count > 1)
		{
			D3D11_TEXTURE2D_DESC capture_desc{};
			back_buffer_ds_tex->GetDesc(&capture_desc);
			ctx->ResolveSubresource(back_buffer_ds_tex, 0, back_buffer_tex, 0, capture_desc.Format);
		}
		else
			ctx->CopySubresourceRegion(back_buffer_ds_tex, 0, 0, 0, 0, back_buffer_tex, 0, nullptr);

		ctx->GenerateMips(back_buffer_ds_full);

		ctx->OMSetRenderTargets(1, &prev_rtv, prev_dsv);
		ctx->PSSetShaderResources(0, 1, &prev_srv);

		if (prev_rtv)
			prev_rtv->Release();
		if (prev_dsv)
			prev_dsv->Release();
		if (prev_srv)
			prev_srv->Release();
	}

	void adapter_dx11::push_pixel_constants(float alpha)
	{
		if (!constant_buffer_ps)
			return;

		D3D11_MAPPED_SUBRESOURCE cb{};
		if (FAILED(ctx->Map(constant_buffer_ps, 0, D3D11_MAP_WRITE_DISCARD, 0, &cb)))
			return;

		auto *dst = static_cast<pixel_constant_buffer *>(cb.pData);
		memset(dst, 0, sizeof(pixel_constant_buffer));
		dst->mvp[0][0] = dst->mvp[1][1] = dst->mvp[2][2] = dst->mvp[3][3] = 1.f;
		dst->tex[0] = draw.display.x * .25f;
		dst->tex[1] = draw.display.y * .25f;
		dst->time = draw.time;
		dst->alpha = alpha;
		ctx->Unmap(constant_buffer_ps, 0);

		ctx->PSSetConstantBuffers(0, 1, &constant_buffer_ps);
	}
} // namespace evo::ren
