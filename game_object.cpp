#include "game_object.h"

GameObject::GameObject()
{
	Init();
}

void GameObject::Init()
{
	position_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	scale_ = gef::Vector4(1.0f, 1.0f, 1.0f);

	active = true;
}

bool GameObject::Update(float frame_time)
{
	position_ += velocity_ * frame_time;

	BuildTransform();
	return true;
}

void GameObject::BuildTransform()
{
	gef::Matrix44 trans;
	trans.SetIdentity();
	trans.Scale(scale_);
	trans.SetTranslation(position_);
	set_transform(trans);
}

