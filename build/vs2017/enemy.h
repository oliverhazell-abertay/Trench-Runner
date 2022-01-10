#pragma once
#include "game_object.h"
#include "graphics/material.h"
#include "primitive_builder.h"
#include <vector>

class Enemy
{
public:
	Enemy(PrimitiveBuilder* prim, gef::Vector4 initPos);

	void Init();
	void CleanUp();
	void Reset();
	bool Update(float frame_time);
	void MoveToTarget(float delta_time);
	void StartMoving(gef::Vector4 target, float moveDuration);
	inline void MarkForDeletion(bool mark) { toBeDeleted = mark; }
	inline bool ToBeDeleted() { return toBeDeleted; }
	void ScaleObjects(gef::Vector4 scale);

	gef::Material material;
	GameObject cockpit_;
	GameObject left_wing_;
	GameObject right_wing_;
	gef::Vector4 position;

	std::vector<GameObject*> gibs_;
	void SpawnGibs();
	void BlowUp();
	int gib_max = 10;
	float gibTimer = 0.0f;
	float gibTimerMax = 2.0f;

	bool alive = true;

private:
	bool moving = false;
	bool toBeDeleted = false;
	gef::Vector4 targetPos;
	gef::Vector4 startPos;
	float lerpTimer = 0.0f;
	float moveSpeed = 1.0f;

	PrimitiveBuilder* primitive_builder_;
};

