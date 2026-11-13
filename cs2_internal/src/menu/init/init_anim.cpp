#include "init_anim.h"

#include "resources/f200.h"
#include "resources/f400.h"
#include "resources/atality.h"

#include <ren/renderer.h>
#include <ren/types/animated_texture.h>
#include <gui/gui.h>
#include <macros.h>

namespace menu
{
	using namespace evo::ren;
	using namespace evo::gui;

	init_anim_t init_anim;

	void init_anim_t::on_adapter_ready()
	{
		draw.textures[FNV1A("f_200")] = std::make_shared<texture>(tex_f200, sizeof(tex_f200));
		draw.textures[FNV1A("f_400")] = std::make_shared<texture>(tex_f400, sizeof(tex_f400));
		draw.textures[FNV1A("atality")] = std::make_shared<texture>(tex_atality, sizeof(tex_atality));

		// copied from gui (pixel cbuffer: projection_matrix, tex, time, alpha)
		draw.shaders[GUI_HASH("init_blur_f")] = std::make_shared<shader>(
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

		toggle = std::make_shared<anim_float>(0.f, .25f);
		letter = std::make_shared<anim_float>(2.f, .3f, ease_out);
		// short hold between the F shrink and the slide (was a 2s breathe loop)
		breathe = std::make_shared<anim_float>(1.f, .25f);
		fly1 = std::make_shared<anim_float>(1.f, .35f, ease_out);
		fly2 = std::make_shared<anim_float>(1.f, .75f, ease_out);

		toggle->direct(1.f);
		toggle->on_finished = [&](const std::shared_ptr<anim<float>> &f)
		{
			if (ending_now)
			{
				init_ready = true;
				return;
			}

			letter->direct(1.f);
			letter->on_finished = [&](const std::shared_ptr<anim<float>> &f)
			{
				breathe_started = true;
				// alpha stays visually full; this only times the 0.25s hold
				breathe->direct(.99f);
				breathe->on_finished = [&](const std::shared_ptr<anim<float>> &c)
				{
					if (ending_now)
						return;

					fly1->direct(0.f);
					ending_now = true;
				};
			};
		};

		fly1->on_finished = [&](const std::shared_ptr<anim<float>> &f)
		{
			if (!ending_now)
				return;

			fly2->direct(0.f);
			fly2->on_finished = [&](const std::shared_ptr<anim<float>> &f)
			{
				toggle->duration = 1.f;
				toggle->direct(0.f);
			};
		};

		adapter_ready = true;
		// No external loading stage calls finish() in this build. Start the
		// completion transition here so the intro plays once instead of waiting
		// forever on the standalone F frame.
		finish();
	}

	void init_anim_t::render()
	{
		if (!adapter_ready)
			return;

		toggle->animate();
		letter->animate();
		breathe->animate();
		fly1->animate();
		fly2->animate();

		auto &d = draw.layers[63];

		d->g.alpha = toggle->value;
		d->g.frag_shader = draw.shaders[GUI_HASH("init_blur_f")]->obj;
		d->g.texture = draw.adapter->get_back_buffer_downsampled();
		d->g.uv_rect = {vec2{0.f, 0.f}, vec2{1.f, 1.f}};
		d->g.only_downsampled = true;
		const auto r_screen = rect().size(draw.display / draw.scale);
		d->add_rect_filled(r_screen, color::white());

		d->g.chained_call = true;
		d->add_rect_filled(r_screen, color::white());

		d->g.only_downsampled = d->g.chained_call = {};
		d->g.frag_shader = d->g.texture = {};
		d->g.uv_rect = {};
		d->g.alpha = 1.f;

		d->add_rect_filled(rect().size(draw.display), color::black().mod_a(.75f).mod_a(toggle->value));

		// draw letter
		if (!ending_now)
		{
			if (!breathe_started)
			{
				auto &f = draw.textures[FNV1A("f_400")];
				d->g.texture = f->obj;

				rect f_rect(draw.display * .5f);
				f_rect = f_rect.size(f->get_size() * letter->value * .5f).translate((f->get_size() * letter->value * .5f * .5f) * -1.f);

				d->skip_rounding = true;
				d->add_rect_filled(f_rect, colors.accent.mod_a(2.f - letter->value));
				d->skip_rounding = false;
				d->g.texture = {};
			}
			else
			{
				auto &f = draw.textures[FNV1A("f_200")];
				d->g.texture = f->obj;
				d->add_rect_filled(rect(draw.display * .5f).size(f->get_size()).translate(f->get_size() * .5f * -1.f), colors.accent.mod_a(breathe->value));
				d->g.texture = {};
			}
		}
		else
		{
			auto &f = draw.textures[FNV1A("f_200")];
			auto &a = draw.textures[FNV1A("atality")];

			const auto fa_w = f->get_size().x * .5f + a->get_size().x * .5f;

			d->g.texture = f->obj;
			d->add_rect_filled(
				rect(draw.display * .5f - vec2{f->get_size().x * .5f, 0.f} - vec2((fa_w - f->get_size().x * .5f) * (1.f - fly1->value), 0.f)).size(f->get_size()).
				translate(vec2(0.f, f->get_size().y * -.5f)), colors.accent.mod_a(toggle->value));

			d->g.texture = a->obj;
			d->add_rect_filled(
				rect(draw.display * .5f - vec2(fa_w - f->get_size().x, -(f->get_size().y * .5f))).size(a->get_size()).translate(vec2(0.f, a->get_size().y * fly2->value)).
																								  translate(vec2(0.f, -a->get_size().y)),
				colors.accent.mod_a(1.f - fly2->value).mod_a(toggle->value));
			d->g.texture = {};
		}
	}

	void init_anim_t::finish()
	{
		init_pre_ready = true;
	}
}
