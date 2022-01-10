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
	rotational_velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	scale_ = gef::Vector4(1.0f, 1.0f, 1.0f);

	active = true;
}

bool GameObject::Update(float frame_time)
{
	position_ += velocity_ * frame_time;
	rotation_ += rotational_velocity_ * frame_time;

	BuildTransform();
	return true;
}

void GameObject::BuildTransform()
{
	gef::Matrix44 trans;
	gef::Matrix44 scale_mtx;
	gef::Matrix44 rotation_mtx, rotation_mtx_x, rotation_mtx_y;
	trans.SetIdentity();
	scale_mtx.SetIdentity();
	rotation_mtx.SetIdentity();
	rotation_mtx_x.SetIdentity();
	rotation_mtx_y.SetIdentity();
	scale_mtx.Scale(scale_);
	rotation_mtx_x.RotationX(rotation_.x());
	rotation_mtx_y.RotationY(rotation_.y());
	rotation_mtx = rotation_mtx_x * rotation_mtx_y;
	trans = scale_mtx * rotation_mtx;
	trans.SetTranslation(position_);
	set_transform(trans);
}

