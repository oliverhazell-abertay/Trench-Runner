#pragma once
#include "game_object.h"
#include "graphics/material.h"
class Enemy : public GameObject
{
public:
	Enemy();

	void Init();
	bool Update(float frame_time);
	void MoveToTarget(float delta_time);
	void StartMoving(gef::Vector4 target, float moveDuration);

	gef::Material material;

private:
	bool moving = false;
	bool toBeDeleted = false;
	gef::Vector4 targetPos;
	gef::Vector4 startPos;
	float lerpTimer = 0.0f;
	float moveSpeed = 1.0f;

};

