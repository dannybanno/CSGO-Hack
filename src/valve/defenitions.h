#pragma once
#include <cstdint>
#include <array>

class Color
{
public:
	Color() :
		r(0), g(0), b(0), a(0) { }

	Color(uint32_t color) :
		color(color) { }

	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) :
		r(r), g(g), b(b), a(a) { }

	Color(const std::array<float, 4U>& color) :
		r(uint8_t(color[0] * 255.f)),
		g(uint8_t(color[1] * 255.f)),
		b(uint8_t(color[2] * 255.f)),
		a(uint8_t(color[3] * 255.f)) { }

	union
	{
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

		uint32_t color;
	};
};