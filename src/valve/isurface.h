#pragma once
#include "../util/memory.h"
#include "defenitions.h"

#ifdef CreateFont1
#undef CreateFont1
#endif

using HFont = unsigned long;

enum FontDrawType
{
	FONT_DRAW_DEFAULT = 0,

	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,

	FONT_DRAW_TYPE_COUNT = 2,
};

enum EFontFlag
{
	FONT_FLAG_NONE,
	FONT_FLAG_ITALIC = 0x001,
	FONT_FLAG_UNDERLINE = 0x002,
	FONT_FLAG_STRIKEOUT = 0x004,
	FONT_FLAG_SYMBOL = 0x008,
	FONT_FLAG_ANTIALIAS = 0x010,
	FONT_FLAG_GAUSSIANBLUR = 0x020,
	FONT_FLAG_ROTARY = 0x040,
	FONT_FLAG_DROPSHADOW = 0x080,
	FONT_FLAG_ADDITIVE = 0x100,
	FONT_FLAG_OUTLINE = 0x200,
	FONT_FLAG_CUSTOM = 0x400,
	FONT_FLAG_BITMAP = 0x800,
};

struct FontVertex
{
	FontVertex() {}
	FontVertex(const CVector& pos, const CVector& coord = CVector(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
	void Init(const CVector& pos, const CVector& coord = CVector(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	CVector m_Position;
	CVector m_TexCoord;
};

using FontHandle = unsigned long;

struct FontCharRenderInfo
{
	// Text pos
	int				x, y;
	// Top left and bottom right
	// This is now a pointer to an array maintained by the surface, to avoid copying the data on the 360
	FontVertex* verts;
	int				textureId;
	int				abcA;
	int				abcB;
	int				abcC;
	int				fontTall;
	FontHandle	currentFont;
	// In:
	FontDrawType	drawType;
	wchar_t			ch;

	// Out
	bool			valid;
	// In/Out (true by default)
	bool			shouldclip;
};

class ISurface
{
public:

	constexpr void DrawSetColor(int r, int g, int b, int a = 255) noexcept
	{
		memory::Call<void>(this, 15, r, g, b, a);
	}

	constexpr void DrawFilledRect(int x, int y, int xx, int yy) noexcept
	{
		memory::Call<void>(this, 16, x, y, xx, yy);
	}

	constexpr void DrawOutlinedRect(int x, int y, int xx, int yy) noexcept
	{
		memory::Call<void>(this, 18, x, y, xx, yy);
	}

	constexpr void DrawLine(int x0, int y0, int x1, int y1)
	{
		memory::Call<void>(this, 19, x0, y0, x1, y1);
	}

	constexpr void DrawCircle(int centerx, int centery, int radius, int r)
	{
		memory::Call<void>(this, 162, centerx, centery, radius, r);
	}

	virtual bool SetFontGlyphSet(HFont font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) = 0;

	virtual void DrawOutlinedCircle(int x, int y, int radius, int segments) = 0;
	

};
