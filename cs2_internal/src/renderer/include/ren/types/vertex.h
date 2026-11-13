#ifndef REN_LINKER_VERTEX_H
#define REN_LINKER_VERTEX_H

namespace evo::ren
{
	extern float vtx_texel;
	extern float vtx_scale;
	// set by the adapter's startup orientation probe when sampled textures come
	// out upside down; every vertex then mirrors its v coordinate
	extern bool vtx_flip_v;

	struct vertex_constant_buffer
	{
		float mvp[4][4];
	};

	// pushed to the pixel stage for custom fragment shaders (blur, etc.)
	// layout mirrors the recovered ui shaders: projection_matrix, tex (= back
	// buffer resolution the taps are offset with), time, alpha
	struct pixel_constant_buffer
	{
		float mvp[4][4];
		float tex[2];
		float time;
		float alpha;
	};

	struct vertex
	{
		vertex(float _x, float _y, float _z, uint32_t _col, float _u, float _v, bool skip = false) :
			x(_x), y(_y), z(_z), col(_col), u(_u), v(_v)
		{
			// Keep subpixel coordinates. Flooring every vertex made corners,
			// icons and glyphs visibly pixelated at fractional positions.
			if (vtx_flip_v)
				v = 1.f - v;

			x -= vtx_texel;
			y -= vtx_texel;

			if (!skip && vtx_scale != 1.f)
			{
				x *= vtx_scale;
				y *= vtx_scale;
			}
		}

		float x, y, z;
		uint32_t col;
		float u, v;
	};

	struct vertex_unfloored
	{
		vertex_unfloored(float _x, float _y, float _z, uint32_t _col, float _u, float _v, bool skip = false) :
			x(_x), y(_y), z(_z), col(_col), u(_u), v(_v)
		{
			if (vtx_flip_v)
				v = 1.f - v;

			x -= vtx_texel;
			y -= vtx_texel;

			if (!skip && vtx_scale != 1.f)
			{
				x *= vtx_scale;
				y *= vtx_scale;
			}
		}

		float x, y, z;
		uint32_t col;
		float u, v;
	};
} // namespace evo::ren

#endif // REN_LINKER_VERTEX_H
