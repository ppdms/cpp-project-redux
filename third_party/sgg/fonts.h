#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include "GL/glew.h"
#endif
#include "sgg/shader.h"
#include <string>
#include <vector>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
// Cached rasterization of one glyph at the font's load resolution. SGG's text
// path re-rasterizes every visible character every frame; the cache pays the
// FreeType cost once per (font, codepoint) and reuses the pixels.
struct CachedGlyph
{
	unsigned int width = 0;
	unsigned int rows = 0;
	float bearingY = 0.0f; // horiBearingY in pixels, like FT_Load_Char reports
	float advanceY = 0.0f; // glyph advance.y in pixels
	std::vector<unsigned char> pixels; // width*rows grayscale bytes
};
#endif

struct Font
{
	std::string fontname;
	FT_Face	face;
	GLuint  font_tex;
#ifdef __EMSCRIPTEN__
	std::unordered_map<unsigned long, CachedGlyph> glyphs;
#endif
};

struct TextRecord
{
	glm::vec2 pos;
	glm::vec2 size;
	std::string text;
	glm::vec4 color1;
	glm::vec4 color2;
	glm::vec2 gradient;
	bool use_gradient;
	glm::mat4 mv;
	glm::mat4 proj;
	Font * font;
};


class FontLib
{
	static FT_Library	m_ft;
	std::unordered_map<std::string, Font>::iterator m_curr_font;
	std::unordered_map<std::string, Font> m_fonts;
	Shader				m_font_shader;
	GLuint				m_font_vbo;
	GLuint				m_font_vao;
	GLuint				m_font_res = 64;
	std::vector<TextRecord> m_content;
	
	glm::vec2	  m_canvas;

	void drawText(TextRecord entry);
	
public:
	bool init();
	void submitText(const TextRecord & text);
	void commitText();
	void setCanvas(glm::vec2 sz);
	bool setCurrentFont(std::string fontname);
	
};

