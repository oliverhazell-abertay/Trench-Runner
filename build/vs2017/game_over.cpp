#include "game_over.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/texture.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <assets/png_loader.h>
#include <graphics/image_data.h>
#include <graphics/font.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <input/sony_controller_input_manager.h>
#include <input/keyboard.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/scene.h>

#define SCREEN_CENTRE_X 480.0f
#define SCREEN_CENTRE_Y 272.0f

GameOver::GameOver(gef::SpriteRenderer* sprite_rend_, gef::InputManager* in_, gef::Font* font) :
	sprite_renderer_(sprite_rend_),
	input_manager_(in_),
	font_(font)
{
	type_ = GAMEOVER;
}

GameOver::~GameOver()
{
}

void GameOver::OnEntry(Type prev_game_state)
{
}

void GameOver::OnExit(Type next_game_state)
{
	signal_to_change = EMPTY;
}

void GameOver::Update(float deltaTime)
{
	// Input
	if (input_manager_)
	{
		input_manager_->Update();
		// controller input
		gef::SonyControllerInputManager* controller_manager = input_manager_->controller_input();
		if (controller_manager)
		{
			if (controller_manager->GetController(0)->buttons_down() == gef_SONY_CTRL_CROSS)
			{
				signal_to_change = MENU
			}
		}
		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			// Move up
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_W))
			{
				signal_to_change = MENU;
			}
		}
	}
}

void GameOver::Render()
{
	sprite_renderer_->Begin();
	if (font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(SCREEN_CENTRE_X, SCREEN_CENTRE_Y, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "GAME OVER");
	}
	sprite_renderer_->End();
}
