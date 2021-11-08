#pragma once
#include "game_state.h"

#include <system/application.h>
#include <graphics/sprite.h>
#include <graphics/mesh_instance.h>
#include "primitive_builder.h"
#include "game_object.h"


namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class Mesh;
	class Scene;
	class InputManager;
}

class Menu : public GameState
{
public:
	Menu(gef::SpriteRenderer* sprite_rend_, gef::InputManager* in_, gef::Font* font);
	~Menu();

	virtual void OnEntry(Type prev_game_state) override;
	virtual void OnExit(Type next_game_state) override;
	virtual void Update(float deltaTime) override;
	virtual void Render() override;
private:
	gef::SpriteRenderer* sprite_renderer_;
	gef::InputManager* input_manager_;
	gef::Font* font_;
};

