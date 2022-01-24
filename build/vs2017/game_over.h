#pragma once
#include "game_state.h"
#include <system/application.h>
#include <graphics/sprite.h>
#include <graphics/mesh_instance.h>
#include "primitive_builder.h"
#include "game_object.h"
#include "menu_object.h"
#include "load_texture.h"


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

class GameOver : public GameState
{
public:
	GameOver(gef::Platform* platform, gef::SpriteRenderer* sprite_rend_, gef::InputManager* in_, gef::Font* font);
	~GameOver();
	void Init();
	void CleanUp();

	virtual void OnEntry(Type prev_game_state) override;
	virtual void OnExit(Type next_game_state) override;
	virtual void Update(float deltaTime) override;
	virtual void Render() override;
private:
	gef::Platform* platform_;
	gef::SpriteRenderer* sprite_renderer_;
	gef::InputManager* input_manager_;
	gef::Font* font_;

	// Background
	void ScrollBG(float delta_time);
	gef::Sprite BG;

	// Screen swipe
	void ScreenSwipe(float delta_time);
	gef::Sprite screen_swipe_sprite;
	bool transistion = false;
	Type swiping_to;

	MenuObject menu_buttons;

	// Logo
	gef::Sprite logo;
	gef::Colour logo_colour;
	bool pulseUp = false;
	void PulseLogo(float delta_time);
};

