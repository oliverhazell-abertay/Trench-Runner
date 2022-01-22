#pragma once
#include "game_object.h"
#include <vector>
#include "primitive_builder.h"

class Player
{
public:
	Player(PrimitiveBuilder* prim);

	void Init();
	bool Update(float delta_time);
	void CleanUp();

	GameObject player_object;

	std::vector<GameObject*> gibs_;
	void SpawnGibs();
	void BlowUp();
	int gib_max = 10;
	float gibTimer = 0.0f;
	float gibTimerMax = 2.0f;

	PrimitiveBuilder* primitive_builder_;

	bool alive = true;
private:

};

