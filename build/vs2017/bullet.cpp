#include "bullet.h"
#include "maths/math_utils.h"
#include <cmath>

void Bullet::Update(float deltaTime)
{
	if (velocity_.Length() != 0.0f)
		isMoving = true;
	else
		isMoving = false;

	if (position_.z() < -1500.0f)
	{
		velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
		position_ = gef::Vector4(0.0f, 0.0f, 0.0f);
		SetActive(false);
	}

	if (isMoving)
		FaceDirectionOfMovement();

	GameObject::Update(deltaTime);
}

void Bullet::FaceDirectionOfMovement()
{
	float theta = atan(velocity_.x() / velocity_.z());
	rotation_.set_y(theta);
	theta = atan(velocity_.y() / velocity_.z());
	rotation_.set_x(theta);
}
