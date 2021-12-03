#include "enemy.h"

Enemy::Enemy()
{
	Init();
}

void Enemy::Init()
{

}

bool Enemy::Update(float delta_time)
{
	GameObject::Update(delta_time);
	return true;
}
