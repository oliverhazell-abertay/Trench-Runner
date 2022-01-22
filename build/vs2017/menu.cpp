#include "Menu.h"
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

Menu::Menu(gef::Platform* platform, gef::SpriteRenderer* sprite_rend_, gef::InputManager* in_, gef::Font* font) :
	platform_(platform),
	sprite_renderer_(sprite_rend_),
	input_manager_(in_),
	font_(font)
{
	type_ = MENU;

	// Buttons
	menu_buttons.Init();
	gef::Sprite tempSprite;
	// Play button
	tempSprite.set_width(120.0f);
	tempSprite.set_height(60.0f);
	tempSprite.set_position(gef::Vector4(SCREEN_CENTRE_X, SCREEN_CENTRE_Y + 120.0f, 0.0f));
	tempSprite.set_texture(CreateTextureFromPNG("button_play_tex.png", *platform));
	UIButton* tempButton = new UIButton(tempSprite);
	menu_buttons.buttons_.push_back(tempButton);
	// Options button
	tempSprite.set_width(120.0f);
	tempSprite.set_height(60.0f);
	tempSprite.set_position(gef::Vector4(SCREEN_CENTRE_X, tempSprite.position().y() + 70.0f, 0.0f));
	tempSprite.set_texture(CreateTextureFromPNG("button_main_menu_tex.png", *platform));
	tempButton = new UIButton(tempSprite);
	menu_buttons.buttons_.push_back(tempButton);

	// Logo init
	logo_colour = gef::Colour(1.0f, 1.0f, 1.0f, 1.0f);
	logo.set_width(576.0f);
	logo.set_height(384.0f);
	logo.set_position(gef::Vector4(SCREEN_CENTRE_X, SCREEN_CENTRE_Y - 100.0f, 0.0f));
	logo.set_texture(CreateTextureFromPNG("logo_tex.png", *platform));

	// BG init
	BG.set_width(1920.0f);
	BG.set_height(544.0f);
	BG.set_position(gef::Vector4(0.0f, SCREEN_CENTRE_Y, 0.0f));
	BG.set_texture(CreateTextureFromPNG("menu_bg.png", *platform));

	// screen swipe init
	screen_swipe_sprite.set_width(1200.0f);
	screen_swipe_sprite.set_height(544.0f);
	screen_swipe_sprite.set_position(gef::Vector4(SCREEN_CENTRE_X + 1200.0f, SCREEN_CENTRE_Y, 0.0f));
	screen_swipe_sprite.set_texture(CreateTextureFromPNG("screenswipe.png", *platform));
}

Menu::~Menu()
{
}

void Menu::OnEntry(Type prev_game_state)
{
}

void Menu::OnExit(Type next_game_state)
{
	signal_to_change = EMPTY;
}

void Menu::Update(float deltaTime)
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
				signal_to_change = GAMERUNNING;
			}
		}
		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			// Start game
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_G))
			{
				transistion = true;
			}
			// Move up
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_W))
			{
				menu_buttons.NextButton();
			}
			// Move down
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_S))
			{
				menu_buttons.PrevButton();
			}
		}
	}
	// Update buttons
	menu_buttons.Update(deltaTime);
	// Scroll Background
	ScrollBG(deltaTime);
	// Pulse logo
	PulseLogo(deltaTime);
	// Scroll screen swipe
	if (transistion)
		ScreenSwipe(deltaTime);
}

void Menu::Render()
{
	sprite_renderer_->Begin();
		// Render background
		sprite_renderer_->DrawSprite(BG);
		// Render buttons
		for (int button_num = 0; button_num < menu_buttons.buttons_.size(); button_num++)
		{
			sprite_renderer_->DrawSprite(menu_buttons.buttons_[button_num]->button_sprite);
		}
		// Render logo
		sprite_renderer_->DrawSprite(logo);
		// Render screenswipe
		//if (transistion)
			sprite_renderer_->DrawSprite(screen_swipe_sprite);
	sprite_renderer_->End();
}

void Menu::ScrollBG(float delta_time)
{
	if(BG.position().x() <= 0.0f)
		BG.set_position(gef::Vector4(BG.position().x() - (500.0f * delta_time) + 960.0f, SCREEN_CENTRE_Y, 0.0f));
	else
		BG.set_position(gef::Vector4(BG.position().x() - (500.0f * delta_time), SCREEN_CENTRE_Y, 0.0f));
}

void Menu::ScreenSwipe(float delta_time)
{
	if (screen_swipe_sprite.position().x() > SCREEN_CENTRE_X - 50.0f)
		screen_swipe_sprite.set_position(gef::Vector4(screen_swipe_sprite.position().x() - (1000.0f * delta_time), SCREEN_CENTRE_Y, 0.0f));
	else
		signal_to_change = GAMERUNNING;
}

void Menu::PulseLogo(float delta_time)
{
	float currentPulse = logo_colour.r;
	float pulseMin = 0.35f;
	float pulseMax = 1.0f;
	float pulseRate = 0.5f;

	if (pulseUp)
	{
		currentPulse += pulseRate * delta_time;
		if (currentPulse >= pulseMax)
		{
			currentPulse = pulseMax;
			pulseUp = false;
		}
	}
	else
	{
		currentPulse -= pulseRate * delta_time;
		if (currentPulse <= pulseMin)
		{
			currentPulse = pulseMin;
			pulseUp = true;
		}
	}
	// Set logo colour based on pulse
	logo_colour = gef::Colour(currentPulse, currentPulse, currentPulse, 1.0f);
	logo.set_colour(logo_colour.GetABGR());
}
