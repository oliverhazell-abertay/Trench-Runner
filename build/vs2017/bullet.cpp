#include "bullet.h"

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

	FaceDirectionOfMovement();

	GameObject::Update(deltaTime);
}

void Bullet::FaceDirectionOfMovement()
{

}
