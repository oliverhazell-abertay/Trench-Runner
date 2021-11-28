#include "game_object.h"

GameObject::GameObject()
{
	Init();
}

void GameObject::Init()
{
	position_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	rotation_ = gef::Vector4(0.0f, 0.0f, 0.0f);
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
	gef::Matrix44 scale_mtx;
	gef::Matrix44 rotation_mtx;
	trans.SetIdentity();
	scale_mtx.SetIdentity();
	rotation_mtx.SetIdentity();
	scale_mtx.Scale(scale_);
	rotation_mtx.RotationY(rotation_.y());
	trans = scale_mtx * rotation_mtx;
	trans.SetTranslation(position_);
	set_transform(trans);
}

