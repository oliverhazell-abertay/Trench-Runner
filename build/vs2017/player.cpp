#include "player.h"

Player::Player()
{

}

void Player::Init()
{

}

bool Player::Update(float delta_time)
{
	player_object.Update(delta_time);

	return true;
}
