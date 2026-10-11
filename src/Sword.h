#pragma once
#include "GameObject.h"
#include "Obstacle.h"
#include "box.h"
#include "Config.h"
#include "sgg_include.h"

class Sword : public GameObject, public Box {
public:
	void update(float dt) override;
	void draw() override;
	void init() override;
	Sword(float x, float y, float w, float h, std::string name);

protected:
	void debugDraw();
};