#pragma once
#include "game_object.h"
class Bullet : public GameObject
{
public:
	void Update(float deltaTime);
	bool GetMoving() { return isMoving; }
private:
	bool isMoving = false;
};

