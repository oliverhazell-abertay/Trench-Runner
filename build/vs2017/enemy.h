#pragma once
#include "game_object.h"
#include "graphics/material.h"
class Enemy : public GameObject
{
public:
	Enemy();

	void Init();
	bool Update(float frame_time);

	gef::Material material;
};

