#include "enemy.h"
#include "maths/math_utils.h"
#include <cmath>

Enemy::Enemy()
{
	Init();
}

void Enemy::Init()
{
	targetPos = position_;
	startPos = position_;
}

bool Enemy::Update(float delta_time)
{
	if (moving)
		MoveToTarget(delta_time);

	GameObject::Update(delta_time);
	return true;
}

void Enemy::StartMoving(gef::Vector4 target, float moveDuration)
{
	startPos = position_;
	targetPos = target;
	moveSpeed = moveDuration;
	moving = true;
}

void Enemy::MoveToTarget(float delta_time)
{
	lerpTimer += (delta_time / moveSpeed);
	if (lerpTimer < 1.0f)
	{
		position_.set_x(gef::Lerp(startPos.x(), targetPos.x(), lerpTimer));
		position_.set_y(gef::Lerp(startPos.y(), targetPos.y(), lerpTimer));
		position_.set_z(gef::Lerp(startPos.z(), targetPos.z(), lerpTimer));
	}
	else
	{
		lerpTimer = 0.0f;
		moving = false;
	}
}

//float Enemy::DistanceBetweenTwoPoints(gef::Vector4 p1, gef::Vector4 p2)
//{
//	float distance = 0.0f;
//	float sqrX = (p2.x() - p1.x()) * (p2.x() - p1.x());
//	float sqrY = (p2.y() - p1.y()) * (p2.y() - p1.y());
//	float sqrZ = (p2.z() - p1.z()) * (p2.z() - p1.z());
//
//	distance = sqrtf(sqrX + sqrY + sqrZ);
//
//	return distance;
//}
