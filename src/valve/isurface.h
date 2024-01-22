#pragma once
#include "../util/memory.h"
#include "defenitions.h"

#ifdef CreateFont1
#undef CreateFont1
#endif

using HFont = unsigned long;

enum FontFeature
{
	FONT_FEATURE_ANTIALIASED_FONTS = 1,
	FONT_FEATURE_DROPSHADOW_FONTS = 2,
	FONT_FEATURE_OUTLINE_FONTS = 6,
};

enum FontDrawType
{
	// Use the "additive" value from the scheme file
	FONT_DRAW_DEFAULT = 0,

	// Overrides
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,

	FONT_DRAW_TYPE_COUNT = 2,
};

enum FontFlags : int
{
	FONTFLAG_NONE = 0,
	FONTFLAG_ITALIC = (1 << 0),
	FONTFLAG_UNDERLINE = (1 << 1),
	FONTFLAG_STRIKEOUT = (1 << 2),
	FONTFLAG_SYMBOL = (1 << 3),
	FONTFLAG_ANTIALIAS = (1 << 4),
	FONTFLAG_GAUSSIANBLUR = (1 << 5),
	FONTFLAG_ROTARY = (1 << 6),
	FONTFLAG_DROPSHADOW = (1 << 7),
	FONTFLAG_ADDITIVE = (1 << 8),
	FONTFLAG_OUTLINE = (1 << 9),
	FONTFLAG_CUSTOM = (1 << 10),
	FONTFLAG_BITMAP = (1 << 11)
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

	//virtual void DrawSetTextFont(HFont font) = 0; // 24
	//virtual void DrawSetTextColor(int r, int g, int b, int a) = 0;
	//virtual void DrawSetTextColor(Color col) = 0;
	//virtual void DrawSetTextPos(int x, int y) = 0;
	//virtual void DrawGetTextPos(int& x, int& y) = 0;
	//virtual void DrawPrintText(const wchar_t* text, int textLen, FontDrawType drawType = FONT_DRAW_DEFAULT) = 0;
	//virtual void DrawUnicodeChar(wchar_t wch, FontDrawType drawType = FONT_DRAW_DEFAULT) = 0;
	//
	//virtual HFont FontCreate() = 0; // Index 71
	//
	//virtual bool SetFontGlyphSet(HFont font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0) = 0;
	//
	//// adds a custom font file (only supports true type font files (.ttf) for now)
	//virtual bool AddCustomFontFile(const char* fontFileName) = 0;
	//
	//// returns the details about the font
	//virtual int GetFontTall(HFont font) = 0;
	//virtual int GetFontAscent(HFont font, wchar_t wch) = 0;
	//virtual bool IsFontAdditive(HFont font) = 0;
	//virtual void GetCharABCwide(HFont font, int ch, int& a, int& b, int& c) = 0;
	//virtual int GetCharacterWidth(HFont font, int ch) = 0;
	//virtual void GetTextSize(HFont font, const wchar_t* text, int& wide, int& tall) = 0;

	void DrawSetTextFont(HFont font)
	{
		//<void(*)(void*, HFont)>(this, 17)(this, font);
		memory::Call<void>(this, 23, font);
	}
	void DrawSetTextColor(int r, int g, int b, int a)
	{
		//GetVirtualFunction<void(*)(void*, int, int, int, int)>(this, 18)(this, r, g, b, a);
		memory::Call<void>(this, 25, r, g, b, a);
	}
	void DrawSetTextColor(Color clr)
	{
		//GetVirtualFunction<void(*)(void*, Color)>(this, 19)(this, clr);
		memory::Call<void>(this, 24, clr);
	}
	void DrawSetTextPos(int x, int y)
	{
		//GetVirtualFunction<void(*)(void*, int, int)>(this, 20)(this, x, y);
		memory::Call<void>(this, 26, x, y);
	}
	//void GetTextSize(unsigned long font, const wchar_t* text, int& wide, int& tall)
	//{
	//	memory::Call<void>(this, 79, font, text, wide, tall);
	//}
	void GetTextSize(HFont font, const wchar_t* text, int& wide, int& tall)
	{
		memory::Call<void>(this, 79, font, text, wide, tall);
	}
	void get_text_size(unsigned long font, const wchar_t* text, int& wide, int& tall) {
		using original_fn = void(__thiscall*)(ISurface*, unsigned long, const wchar_t*, int&, int&);
		return (*(original_fn**)this)[79](this, font, text, wide, tall);
	}
	void DrawPrintText(const wchar_t* text, int texeLen, FontDrawType drawType = FONT_DRAW_DEFAULT)
	{
		//GetVirtualFunction<void(*)(void*, const wchar_t*, int, FontDrawType_t)>(this, 22)(this, text, texeLen, drawType);
		memory::Call<void>(this, 28, text, texeLen, drawType);
	}
	HFont CreateFont1()
	{
		//return GetVirtualFunction<HFont(*)(void*)>(this, 66)(this);
		return memory::Call<HFont>(this, 71);
	}
	void SetFontGlyphSet(HFont& font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0)
	{
		//GetVirtualFunction<void(*)(void*, HFont&, const char*, int, int, int, int, int, int, int)>(this, 67)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
		memory::Call<void>(this, 72, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
	}
	
};
