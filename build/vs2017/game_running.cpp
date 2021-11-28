#include "game_running.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <graphics/font.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <input/sony_controller_input_manager.h>
#include <input/keyboard.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/scene.h>
#include <stdlib.h>
//#include <sony_sample_framework.h>
//#include <sony_tracking.h>
#include <graphics/renderer_3d.h>
#include <graphics/render_target.h>
#include <graphics/image_data.h>
#include <graphics/material.h>

#define SCREEN_CENTRE_X 480.0f
#define SCREEN_CENTRE_Y 272.0f
#define ALL_DUCKS int i = 0; i < ducks_.size(); i++

GameRunning::GameRunning(gef::Platform* platform, gef::SpriteRenderer* sprite_rend_, gef::Renderer3D* rend_3d_, gef::InputManager* in_) :
	platform_(platform),
	sprite_renderer_(sprite_rend_),
	renderer_3d_(rend_3d_),
	input_manager_(in_),
	font_(NULL),
	model_scene_(NULL),
	mesh_(NULL)
{
	type_ = GAMERUNNING;
	srand(time(NULL));
	InitFont();

	primitive_builder_ = new PrimitiveBuilder(*platform_);

	// Player Init
	bullet_.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(0.1f, 0.1f, 1.0f)));
	gef::Vector4 translation(0.0f, 0.0f, 0.0f);
	bullet_.position_ = translation;
	bullet_.velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);

	// Init ducks
	for (int i = 0; i < max_ducks; i++)
	{
		Duck tempDuck;
		tempDuck.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(1.0f, 1.0f, 1.0f)));
		gef::Vector4 translation_(0.0f, 0.0f, 0.0f);
		tempDuck.position_ = translation_;
		ducks_.push_back(tempDuck);
	}

	// Init duck spawner
	const float grass_size_ = 300.0f;
	grass_.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(1.0f, 1.0f, 1.0f)));
	gef::Vector4 grass_translation_(0.0f, -100.0f, 0.0f);
	grass_.position_ = grass_translation_;
	grass_.scale_ = gef::Vector4(grass_size_, 1.0f, grass_size_);

	// Init cursor
	crosshair_ = CreateTextureFromPNG("crosshair.png", *platform_);
	cursor_.set_texture(crosshair_);
	cursor_.set_height(100.0f);
	cursor_.set_width(100.0f);
	cursor_.set_position(gef::Vector4(SCREEN_CENTRE_X, SCREEN_CENTRE_Y, 0.0f));
	gef::Colour crosshair_colour_(0.0f, 1.0f, 0.0f, 1.0f);
	cursor_.set_colour(crosshair_colour_.GetABGR());
}

GameRunning::~GameRunning()
{
	CleanUp();
}

void GameRunning::OnEntry(Type prev_game_state)
{
	// if coming from menu, restart game
	if (prev_game_state != PAUSE)
	{
		score = 0;
		wave_count = 0;
	}
	SetupCamera();
	SetupLights();
}

void GameRunning::OnExit(Type next_game_state)
{
	signal_to_change = EMPTY;
}

void GameRunning::CleanUp()
{
	delete font_;
	font_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	delete model_scene_;
	model_scene_ = NULL;

	delete mesh_;
	mesh_ = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;
}

void GameRunning::Update(float delta_time)
{
	Input(delta_time);
	UpdateDucks(delta_time);

	// floor update
	grass_.Update(delta_time);
	// Player update
	bullet_.Update(delta_time);

	// Win condition
	if (score > 10 || wave_count >= 5)
	{
		signal_to_change = GAMEOVER;
	}
}

void GameRunning::Render()
{
	//
	// Render 3D scene
	//

	// SET VIEW AND PROJECTION MATRIX HERE
	gef::Matrix44 projection_matrix;
	gef::Matrix44 view_matrix;

	projection_matrix = platform_->PerspectiveProjectionFov(camera_fov_, (float)platform_->width() / (float)platform_->height(), near_plane_, far_plane_);
	view_matrix.LookAt(camera_eye_, camera_lookat_, camera_up_);
	renderer_3d_->set_projection_matrix(projection_matrix);
	renderer_3d_->set_view_matrix(view_matrix);

	// Begin rendering 3D meshes
	renderer_3d_->Begin();
		// Draw floor
		renderer_3d_->DrawMesh(grass_);
		// Draw ducks
		for (ALL_DUCKS)
		{
			if (ducks_[i].GetActive() == true)
			{
				renderer_3d_->DrawMesh(ducks_.at(i));
			}
		}
		//Draw player
		if (bullet_.GetActive())
		{
			renderer_3d_->set_override_material(&primitive_builder_->red_material());
			renderer_3d_->DrawMesh(bullet_);
		}
	renderer_3d_->set_override_material(NULL);
	renderer_3d_->End();

	// Draw UI
	sprite_renderer_->Begin(false);
		DrawHUD();
	sprite_renderer_->End();
}

void GameRunning::InitFont()
{
	font_ = new gef::Font(*platform_);
	font_->Load("comic_sans");
}

void GameRunning::DrawHUD()
{
	/*if (font_)
	{
		if(next_wave)
			font_->RenderText(sprite_renderer_, gef::Vector4(SCREEN_CENTRE_X, 100.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "NEXT WAVE IN: %.1f", (wave_delay - wave_timer));
	}*/
	// Draw cursor
	sprite_renderer_->DrawSprite(cursor_);
}

void GameRunning::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void GameRunning::SetupCamera()
{
	// initialise the camera settings
	camera_eye_ = gef::Vector4(5.0f, 5.0f, 500.0f);
	camera_lookat_ = gef::Vector4(0.0f, 0.0f, 0.0f);
	camera_up_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_fov_ = gef::DegToRad(45.0f);
	near_plane_ = 0.01f;
	far_plane_ = 100.f;
}

gef::Mesh* GameRunning::GetFirstMesh(gef::Scene* scene)
{
	gef::Mesh* mesh = NULL;

	if (scene)
	{
		// now check to see if there is any mesh data in the file, if so lets create a mesh from it
		if (scene->mesh_data.size() > 0)
			mesh = scene->CreateMesh(*platform_, scene->mesh_data.front());
	}

	return mesh;
}

void GameRunning::ReadSceneAndAssignFirstMesh(const char* filename, gef::Scene** scene, gef::Mesh** mesh)
{
	// create a new scene object and read in the data from the file
	// no meshes or materials are created yet
	// we're not making any assumptions about what the data may be loaded in for
	gef::Scene* scn = new gef::Scene;
	scn->ReadSceneFromFile(*platform_, filename);

	// we do want to render the data stored in the scene file so lets create the materials from the material data present in the scene file
	scn->CreateMaterials(*platform_);

	// if there is mesh data in the scene, create a mesh to draw from the first mesh
	*mesh = GetFirstMesh(scn);
	*scene = scn;
}

bool GameRunning::IsColliding_SphereToSphere(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2)
{
	gef::Sphere sphere1 = meshInstance1.mesh()->bounding_sphere();
	gef::Sphere sphere2 = meshInstance2.mesh()->bounding_sphere();

	gef::Sphere sphere1_transformed = sphere1.Transform(meshInstance1.transform());
	gef::Sphere sphere2_transformed = sphere2.Transform(meshInstance2.transform());

	gef::Vector4 offset = sphere1_transformed.position() - sphere2_transformed.position();
	float distance = sqrtf(offset.x() * offset.x() + offset.y() * offset.y() + offset.z() * offset.z());
	float combined_radii = sphere1.radius() + sphere2.radius();

	return distance < combined_radii;
}

bool GameRunning::IsColliding_AABBToAABB(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2)
{
	gef::Aabb Aabb1 = meshInstance1.mesh()->aabb();
	gef::Aabb Aabb2 = meshInstance2.mesh()->aabb();

	gef::Aabb Aabb1_t = Aabb1.Transform(meshInstance1.transform());
	gef::Aabb Aabb2_t = Aabb2.Transform(meshInstance2.transform());

	if (Aabb1_t.max_vtx().x() > Aabb2_t.min_vtx().x() &&
		Aabb1_t.min_vtx().x() < Aabb2_t.max_vtx().x() &&
		Aabb1_t.max_vtx().y() > Aabb2_t.min_vtx().y() &&
		Aabb1_t.min_vtx().y() < Aabb2_t.max_vtx().y() &&
		Aabb1_t.max_vtx().z() > Aabb2_t.min_vtx().z() &&
		Aabb1_t.min_vtx().z() < Aabb2_t.max_vtx().z()
		)
	{
		return true;
	}

	return false;
}

void GameRunning::SpawnWave()
{
	for (ALL_DUCKS)
	{
		ducks_.at(i).Spawn(gef::Vector4(0.5f, 0.3f, 0.0f));
	}
	active_duck_count = ducks_.size();
	wave_count++;
}

void GameRunning::Input(float delta_time)
{
	// Input
	if (input_manager_)
	{
		input_manager_->Update();

		// controller input
		gef::SonyControllerInputManager* controller_manager = input_manager_->controller_input();
		if (controller_manager)
		{
			// Shoot
			if (controller_manager->GetController(0)->buttons_down() == gef_SONY_CTRL_R2)
			{
				bullet_.SetActive(true);
				bullet_.position_ = gef::Vector4(0.0f, 0.0f, 0.0f);
				bullet_.velocity_ = gef::Vector4(0.0f, 0.0f, -10.0f);
			}
			// Pause
			if (controller_manager->GetController(0)->buttons_down() == gef_SONY_CTRL_START)
			{
				signal_to_change = PAUSE;
			}
		}

		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			int sensitivity = 500;
			if (keyboard->IsKeyDown(gef::Keyboard::KC_LSHIFT))
				sensitivity = 100;
			// Get cursor current position
			gef::Vector4 temp_pos_ = cursor_.position();
			// Move up
			if (keyboard->IsKeyDown(gef::Keyboard::KC_W))
			{
				temp_pos_.set_y(temp_pos_.y() - (sensitivity * delta_time));
				/*if (!bullet_.GetMoving())
				{
					bullet_.position_.set_y(bullet_.position_.y() + (sensitivity * delta_time));
				}*/
			}
			// Move left
			if (keyboard->IsKeyDown(gef::Keyboard::KC_A))
			{
				temp_pos_.set_x(temp_pos_.x() - (sensitivity * delta_time));
				/*if (!bullet_.GetMoving())
				{
					bullet_.position_.set_x(bullet_.position_.x() - (sensitivity * delta_time));
				}*/
			}
			// Move down
			if (keyboard->IsKeyDown(gef::Keyboard::KC_S))
			{
				temp_pos_.set_y(temp_pos_.y() + (sensitivity * delta_time));
				/*if (!bullet_.GetMoving())
				{
					bullet_.position_.set_y(bullet_.position_.y() - (sensitivity * delta_time));
				}*/
			}
			// Move right
			if (keyboard->IsKeyDown(gef::Keyboard::KC_D))
			{
				temp_pos_.set_x(temp_pos_.x() + (sensitivity * delta_time));
				/*if (!bullet_.GetMoving())
				{
					bullet_.position_.set_x(bullet_.position_.x() + (sensitivity * delta_time));
				}*/
			}
			// Set cursor to new position based on input
			cursor_.set_position(temp_pos_);

			// Shoot
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) //&& !bullet_.GetMoving())
			{
				bullet_.SetActive(true);
				CastRayFromCamera();
			}
			// Pause
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_P))
			{
				signal_to_change = PAUSE;
			}
		}
	}
}

void GameRunning::UpdateDucks(float delta_time)
{
	// If ducks are still alive
	if (active_duck_count > 0)
		active_duck_count = ducks_.size();
	// Ducks update
	for (ALL_DUCKS)
	{
		ducks_.at(i).Update(delta_time);
		// Duck has been shot
		if (IsColliding_AABBToAABB(bullet_, ducks_[i]) == true && ducks_.at(i).GetActive() == true)
		{
			score++;
			bullet_.SetActive(false);
			ducks_.at(i).SetActive(false);
		}
		if (!ducks_.at(i).GetActive())
			active_duck_count--;
	}
	// All ducks inactive, start next wave countdown
	if (active_duck_count <= 0)
	{
		next_wave = true;
		wave_timer += delta_time;
		if (wave_timer > wave_delay)
		{
			next_wave = false;
			SpawnWave();
			wave_timer = 0.0f;
		}
	}
}

// https://antongerdelan.net/opengl/raycasting.html
void GameRunning::CastRayFromCamera()
{
	gef::Vector4 startPoint;
	gef::Vector4 direction;

	gef::Vector2 ndc;

	float hw = platform_->width() * 0.5f;
	float hh = platform_->height() * 0.5f;

	ndc.x = (static_cast<float>(cursor_.position().x()) - hw) / hw;
	ndc.y = (hh - static_cast<float>(cursor_.position().y())) / hh;

	gef::Matrix44 projectionInverse;
	projectionInverse.Inverse(renderer_3d_->view_matrix() * renderer_3d_->projection_matrix());

	gef::Vector4 nearPoint, farPoint;

	nearPoint = gef::Vector4(ndc.x, ndc.y, 0.01f, 1.0f).TransformW(projectionInverse);
	farPoint = gef::Vector4(ndc.x, ndc.y, 100.0f, 1.0f).TransformW(projectionInverse);

	nearPoint /= nearPoint.w();
	farPoint /= farPoint.w();

	startPoint = gef::Vector4(nearPoint.x(), nearPoint.y(), nearPoint.z());
	direction = nearPoint - farPoint;
	direction.Normalise();

	bullet_.position_ = startPoint;
	bullet_.velocity_ = direction * 10;
}