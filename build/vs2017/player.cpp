#include "player.h"

#define ALL_GIBS int gib_num = 0; gib_num < gibs_.size(); gib_num++

Player::Player(PrimitiveBuilder* prim)
{
	primitive_builder_ = prim;

	Init();
}

void Player::Init()
{
	// Gibs init
	for (int gib_num = 0; gib_num < gib_max; gib_num++)
	{
		GameObject* tempGib;
		tempGib = new GameObject;
		tempGib->set_mesh(primitive_builder_->GetDefaultCubeMesh());
		tempGib->SetActive(false);
		gibs_.push_back(tempGib);
	}
}

void Player::CleanUp()
{
	for (ALL_GIBS)
	{
		delete gibs_[gib_num];
		gibs_[gib_num] = NULL;
	}
}

bool Player::Update(float delta_time)
{
	player_object.Update(delta_time); 
	
	if (!alive)
	{
		// Update gibs
		gibTimer += delta_time;
		for (ALL_GIBS)
		{
			// If gibs are less than a second old, update them
			if (gibTimer < gibTimerMax)
				gibs_[gib_num]->Update(delta_time);
			else
			{
				gibs_[gib_num]->SetActive(false);
				gibTimer = 0;
			}
		}
	}

	return true;
}

void Player::BlowUp(int dir)
{
	alive = false;
	SpawnGibs(dir);
	player_object.velocity_ = gef::Vector4(0.0f, 0.0f, 50.0f * dir);
}

void Player::SpawnGibs(int dir)
{
	for (ALL_GIBS)
	{
		gibs_[gib_num]->SetActive(true);
		gibs_[gib_num]->position_ = gef::Vector4(player_object.position_.x(), player_object.position_.y(), player_object.position_.z() - 2.0f);
		gibs_[gib_num]->velocity_ = gef::Vector4((float)(rand() % 60 - 30), (float)(rand() % 60 - 30), 50.0f * dir);
	}
}
