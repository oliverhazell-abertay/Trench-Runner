#include "enemy.h"
#include "maths/math_utils.h"
#include <cmath>

#define ALL_GIBS int gib_num = 0; gib_num < gibs_.size(); gib_num++

Enemy::Enemy(PrimitiveBuilder* prim, gef::Vector4 initPos)
{
	primitive_builder_ = prim;
	position = initPos;
	Init();
}

void Enemy::CleanUp()
{
	for (ALL_GIBS)
	{
		delete gibs_[gib_num];
		gibs_[gib_num] = NULL;
	}
}

void Enemy::Init()
{
	// Cockpit init
	cockpit_.set_mesh(primitive_builder_->GetDefaultSphereMesh());
	cockpit_.position_ = position;
	cockpit_.scale_ = gef::Vector4(10.0f, 10.0f, 10.0f);

	// Left wing init
	left_wing_.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	left_wing_.position_ = position;
	left_wing_.scale_ = gef::Vector4(1.0f, 15.0f, 10.0f);
	left_wing_.position_.set_x(position.x() - (cockpit_.scale_.x() / 2) - (left_wing_.scale_.x() / 2));

	// Right wing init
	right_wing_.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	right_wing_.position_ = position;
	right_wing_.scale_ = gef::Vector4(1.0f, 15.0f, 10.0f);
	right_wing_.position_.set_x(position.x() + (cockpit_.scale_.x() / 2) + (right_wing_.scale_.x() / 2));
	
	material = primitive_builder_->blue_material();

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

void Enemy::Reset()
{
	cockpit_.velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	left_wing_.velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	right_wing_.velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);

	cockpit_.rotational_velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	left_wing_.rotational_velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	right_wing_.rotational_velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);

	left_wing_.rotation_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	right_wing_.rotation_ = gef::Vector4(0.0f, 0.0f, 0.0f);
}

bool Enemy::Update(float delta_time)
{
	if (alive)
	{
		// Update positions
		cockpit_.position_ = position;
		left_wing_.position_ = position;
		right_wing_.position_ = position;

		// Adjust wing positions
		left_wing_.position_.set_x(position.x() - (cockpit_.scale_.x() / 2) - (left_wing_.scale_.x() / 2));
		right_wing_.position_.set_x(position.x() + (cockpit_.scale_.x() / 2) + (right_wing_.scale_.x() / 2));

		// If not at target, keep moving
		if (moving)
			MoveToTarget(delta_time);
		// If at target, find random new target
		if (!moving)
		{
			gef::Vector4 randTarget(rand() % 400 - 200, rand() % 200 - 100, 300.0f);
			StartMoving(randTarget, 3.0f);
		}
	}

	// Update objects
	cockpit_.Update(delta_time);
	left_wing_.Update(delta_time);
	right_wing_.Update(delta_time);

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

void Enemy::ScaleObjects(gef::Vector4 scale)
{
	// Scale cockpit
	cockpit_.scale_ = gef::Vector4(cockpit_.scale_.x() * scale.x(), 
										cockpit_.scale_.y() * scale.y(),
											cockpit_.scale_.z() * scale.z());
	// Scale left wing
	left_wing_.scale_ = gef::Vector4(left_wing_.scale_.x() * scale.x(),
										left_wing_.scale_.y() * scale.y(),
											left_wing_.scale_.z() * scale.z());

	// Scale right wing
	right_wing_.scale_ = gef::Vector4(right_wing_.scale_.x() * scale.x(),
										right_wing_.scale_.y() * scale.y(),
											right_wing_.scale_.z() * scale.z());
}

void Enemy::StartMoving(gef::Vector4 target, float moveDuration)
{
	startPos = position;
	targetPos = target;
	moveSpeed = moveDuration;
	moving = true;
}

void Enemy::MoveToTarget(float delta_time)
{
	lerpTimer += (delta_time / moveSpeed);
	if (lerpTimer < 1.0f)
	{
		position.set_x(gef::Lerp(startPos.x(), targetPos.x(), lerpTimer));
		position.set_y(gef::Lerp(startPos.y(), targetPos.y(), lerpTimer));
		position.set_z(gef::Lerp(startPos.z(), targetPos.z(), lerpTimer));
	}
	else
	{
		lerpTimer = 0.0f;
		moving = false;
	}
}

void Enemy::BlowUp()
{
	alive = false;
	SpawnGibs();

	cockpit_.velocity_ = gef::Vector4(0.0f, 0.0f, 100.0f);
	left_wing_.velocity_ = gef::Vector4(rand() % 30 - 30, rand() % 30, 100.0f);
	left_wing_.rotational_velocity_ = gef::Vector4(rand() % 5 - 2, rand() % 5 - 2, rand() % 5 - 2);
	right_wing_.velocity_ = gef::Vector4(rand() % 30, rand() % 30, 100.0f);
	right_wing_.rotational_velocity_ = gef::Vector4(rand() % 5 - 2, rand() % 5 - 2, rand() % 5 - 2);
}

void Enemy::SpawnGibs()
{
	for (ALL_GIBS)
	{
		gibs_[gib_num]->SetActive(true);
		gibs_[gib_num]->position_ = gef::Vector4(position.x(), position.y(), position.z() - cockpit_.scale_.z());
		gibs_[gib_num]->velocity_ = gef::Vector4((float)(rand() % 60 - 30), (float)(rand() % 60 - 30), 300.0f);
	}
}
