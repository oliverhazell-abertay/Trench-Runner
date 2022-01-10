#pragma once
#include <graphics/mesh_instance.h>

class GameObject : public gef::MeshInstance
{
public:
	GameObject();

	void Init();
	bool Update(float frame_time);

	gef::Vector4 position_;
	gef::Vector4 velocity_;
	gef::Vector4 rotational_velocity_;
	gef::Vector4 scale_;
	gef::Vector4 rotation_;
	void BuildTransform();
	bool GetActive() { return active; }
	void SetActive(bool nextActive) { active = nextActive; }
private:
	bool active;
};

