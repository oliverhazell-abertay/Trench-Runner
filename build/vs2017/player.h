#pragma once
#include "game_object.h"

class Player
{
public:
	Player();

	void Init();
	bool Update(float delta_time);

	GameObject player_object;
private:

};

