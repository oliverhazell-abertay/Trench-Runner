#pragma once
#include "game_object.h"
#include "maths/math_utils.h"

class Duck : public GameObject
{
public:
	void Update(float frame_time);
	void Fly(float frame_time);
	void Spawn(gef::Vector4 init_velocity_);
	bool isFlying = false;
private:
	gef::Vector4 first_velocity_;
	float flying_time_ = 0.0f;
};

