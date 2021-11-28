#pragma once
#include "game_object.h"
class Bullet : public GameObject
{
public:
	void Update(float deltaTime);
	void FaceDirectionOfMovement();
	bool GetMoving() { return isMoving; }
private:
	bool isMoving = false;
};

