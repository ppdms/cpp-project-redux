#include "Sword.h"
#include "sgg_include.h"
#include "util.h"
#include <iostream>

void Sword::update(float dt)
{
}

void Sword::debugDraw()
{
	graphics::Brush debug_brush;
	debug_brush.fill_color[0] = 0.15f;
	debug_brush.fill_color[1] = 0.7f;
	debug_brush.fill_color[2] = 0.5f;

	graphics::drawRect(m_pos_x, m_pos_y, m_width, m_height, debug_brush);

	graphics::setFont(std::string(ASSET_PATH) + "JetBrainsMono-Thin.ttf");
}

void Sword::draw()
{
	if (m_state->m_debugging)
		debugDraw();
}

void Sword::init()
{
}

Sword::Sword(float x, float y, float w, float h, std::string name) : Box(x, y, w, h), GameObject(name)
{
}
