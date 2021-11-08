#include "duck.h"
#include <cmath>

void Duck::Update(float frame_time)
{
	if (isFlying)
	{
		Fly(frame_time);
	}

	if (position_.y() < -10.0f)
	{
		flying_time_ = 0.0f;
		isFlying = false;
		velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
		position_ = gef::Vector4(0.0f, 0.0f, -1500.0f);
		SetActive(false);
	}

	GameObject::Update(frame_time);
}

void Duck::Fly(float frame_time)
{
	// Velocity
	// x = v0x.cos(theta)
	// y = v0y.sin(theta) - g.t
	// z = v0z.cos(theta)
	float x, y;
	float v0x, v0y;
	float g = 9.81f;
	float theta = 45.0f;
	theta = gef::DegToRad(theta);
	float cos_theta = cos(theta);
	float sin_theta = sin(theta);
	theta = gef::RadToDeg(theta);
	flying_time_ += frame_time;
	float t = flying_time_;

	v0x = first_velocity_.x();
	v0y = first_velocity_.y();

	x = v0x;
	velocity_.set_x(x);

	y = (v0y * sin_theta) - (g * t);
	velocity_.set_y(y);
}

void Duck::Spawn(gef::Vector4 init_velocity_)
{
	gef::Vector4 translation_2_(rand() % 2000 - 1000, rand() % 100 - 800, -1500.0f);
	position_ = translation_2_;
	first_velocity_ = init_velocity_;
	isFlying = true;
	SetActive(true);
}
