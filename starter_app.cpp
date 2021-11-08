#include "starter_app.h"
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

StarterApp::StarterApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	font_(NULL),
	renderer_3d_(NULL)
{
}

void StarterApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	renderer_3d_ = gef::Renderer3D::Create(platform_);
	input_manager_ = gef::InputManager::Create(platform_);

	InitFont();
	SetupCamera();
	SetupLights();
	
	game_states_[0] = new Menu(sprite_renderer_, input_manager_, font_);
	game_states_[1] = new GameRunning(&platform_, sprite_renderer_, renderer_3d_, input_manager_);
	game_states_[2] = new Paused(sprite_renderer_, input_manager_, font_);
	game_states_[3] = new GameOver(sprite_renderer_, input_manager_, font_);
}

void StarterApp::CleanUp()
{
	delete primitive_builder_;
	primitive_builder_ = NULL;

	CleanUpFont();

	delete input_manager_;
	input_manager_ = NULL;

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete game_states_[0];
	game_states_[0] = NULL;

	delete game_states_[1];
	game_states_[1] = NULL;
}

bool StarterApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;
	
	// Check if current game state is signalling to change
	if (game_states_[current_game_state_]->signal_to_change != EMPTY)
	{
		for (int i = 0; i < total_game_states; ++i)
		{
			if (game_states_[i]->GetType() == game_states_[current_game_state_]->signal_to_change)
			{
				ChangeGameState(i);
			}
		}
	}
	// Player update
	game_states_[current_game_state_]->Update(frame_time);
	
	return true;
}

void StarterApp::Render()
{
	gef::Matrix44 projection_matrix;
	gef::Matrix44 view_matrix;

	projection_matrix = platform_.PerspectiveProjectionFov(camera_fov_, (float)platform_.width() / (float)platform_.height(), near_plane_, far_plane_);
	view_matrix.LookAt(camera_eye_, camera_lookat_, camera_up_);
	renderer_3d_->set_projection_matrix(projection_matrix);
	renderer_3d_->set_view_matrix(view_matrix);

	game_states_[current_game_state_]->Render();

	// setup the sprite renderer, but don't clear the frame buffer
	// draw 2D sprites here
	sprite_renderer_->Begin(false);
		DrawHUD();
	sprite_renderer_->End();
}
void StarterApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void StarterApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void StarterApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);
	}
}

void StarterApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void StarterApp::SetupCamera()
{
	// initialise the camera settings
	camera_eye_ = gef::Vector4(0.0f, 0.0f, 1000.0f);
	camera_lookat_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	camera_up_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_fov_ = gef::DegToRad(45.0f);
	near_plane_ = 0.01f;
	far_plane_ = 5000.f;
}

void StarterApp::ChangeGameState(int next_game_state_num_)
{
	game_states_[current_game_state_]->OnExit(game_states_[next_game_state_num_]->GetType());
	game_states_[next_game_state_num_]->OnEntry(game_states_[current_game_state_]->GetType());
	current_game_state_ = next_game_state_num_;
}