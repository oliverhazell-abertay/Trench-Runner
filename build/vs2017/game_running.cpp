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
#include <sony_sample_framework.h>
#include <sony_tracking.h>
#include <graphics/renderer_3d.h>
#include <graphics/render_target.h>
#include <graphics/image_data.h>

#define SCREEN_CENTRE_X 480.0f
#define SCREEN_CENTRE_Y 272.0f
#define MARKER_SIZE 0.059f
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
	InitAR();

	primitive_builder_ = new PrimitiveBuilder(*platform_);

	// Player Init
	bullet_.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(MARKER_SIZE / 4.0f, MARKER_SIZE / 4.0f, MARKER_SIZE / 4.0f));
	gef::Vector4 translation(0.0f, 0.0f, 0.0f);
	bullet_.position_ = translation;
	bullet_.velocity_ = gef::Vector4(0.0f, 0.0f, 0.0f);

	// Init ducks
	for (int i = 0; i < max_ducks; i++)
	{
		Duck tempDuck;
		tempDuck.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(MARKER_SIZE / 6.0f, MARKER_SIZE / 6.0f, MARKER_SIZE / 6.0f)));
		gef::Vector4 translation_(0.0f, 0.0f, 0.0f);
		tempDuck.position_ = translation_;
		ducks_.push_back(tempDuck);
	}

	// Init duck spawner
	const float grass_size_ = 3000.0f;
	grass_.set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(MARKER_SIZE / 2.0f, MARKER_SIZE / 2.0f, MARKER_SIZE / 2.0f)));
	gef::Vector4 grass_translation_(0.0f, 0.0f, 0.0f);
	grass_.position_ = grass_translation_;
	grass_.scale_ = gef::Vector4(grass_size_, 1.0f, 0.059f);

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

	// use the tracking library to try and find markers
	smartUpdate(dat->currentImage);
	//check to see if a particular marker can be found
	if (sampleIsMarkerFound(1))
	{
		// marker is being tracked, get its transform
		gef::Matrix44 marker_transform;
		sampleGetTransform(1, &marker_transform);
		// set the transform of the 3D mesh instance to draw on top of the marker
		grass_.set_transform(marker_transform);
		for (ALL_DUCKS)
		{
			// if duck is waiting to spawn, reset position to marker
			if (!ducks_[i].isFlying)
				ducks_[i].set_transform(marker_transform);
			// ready to spawn
		}
	}
	sampleUpdateEnd(dat);

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
	AppData* dat = sampleRenderBegin();

	//
	// Render the camera feed
	//

	// REMEMBER AND SET THE PROJECTION MATRIX HERE
	sprite_renderer_->set_projection_matrix(camera_feed_matrix_);

	sprite_renderer_->Begin(true);

	// DRAW CAMERA FEED SPRITE HERE
	// check there is data present for the camera image before trying to draw it
	if (dat->currentImage)
	{
		camera_feed_tex_.set_texture(dat->currentImage->tex_yuv);
		sprite_renderer_->DrawSprite(camera_image_sprite_);
	}

	sprite_renderer_->End();

	//
	// Render 3D scene
	//

	// SET VIEW AND PROJECTION MATRIX HERE
	renderer_3d_->set_projection_matrix(projection_3d_);
	gef::Matrix44 view_matrix_;
	view_matrix_.SetIdentity();
	renderer_3d_->set_view_matrix(view_matrix_);

	// Begin rendering 3D meshes, don't clear the frame buffer
	renderer_3d_->Begin(false);
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
			renderer_3d_->DrawMesh(bullet_);
	renderer_3d_->End();

	// Draw UI
	sprite_renderer_->Begin(false);
		DrawHUD();
	sprite_renderer_->End();

	sampleRenderEnd();
}

void GameRunning::InitFont()
{
	font_ = new gef::Font(*platform_);
	font_->Load("comic_sans");
}

void GameRunning::DrawHUD()
{
	if (font_)
	{
		if(next_wave)
			font_->RenderText(sprite_renderer_, gef::Vector4(SCREEN_CENTRE_X, 100.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "NEXT WAVE IN: %.1f", (wave_delay - wave_timer));
	}
	// Draw cursor
	sprite_renderer_->DrawSprite(cursor_);
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
		ducks_.at(i).Spawn(gef::Vector4(0.5f, 0.3f, 0.0f);
	}
	active_duck_count = ducks_.size();
	wave_count++;
}

void GameRunning::InitAR()
{
	// initialise sony framework
	sampleInitialize();
	smartInitialize();

	// reset marker tracking
	AppData* dat = sampleUpdateBegin();
	smartTrackingReset();
	sampleUpdateEnd(dat);

	// Set orthographic view
	camera_feed_matrix_ = platform_->OrthographicFrustum(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	// Calculate vertical image scale factor
	cameraAspectRatio = 640.0f / 480.0f;
	displayAspectRatio = 960.0f / 544.0f;
	verticalScaleFactor = displayAspectRatio / cameraAspectRatio;

	// Set projection matrix for 3d
	projection_3d_.SetIdentity();
	projection_3d_ = platform_->PerspectiveProjectionFov(SCE_SMART_IMAGE_FOV, (float)SCE_SMART_IMAGE_WIDTH / (float)SCE_SMART_IMAGE_HEIGHT, 0.1f, 100.0f);
	gef::Matrix44 scale_temp_;
	scale_temp_.SetIdentity();
	scale_temp_.SetRow(1, gef::Vector4(0.0f, verticalScaleFactor, 0.0f));
	projection_3d_ = projection_3d_ * scale_temp_;

	// Init sprite for camera sprite
	camera_image_sprite_.set_position(gef::Vector4(0.0f, 0.0f, 1.0f));
	camera_image_sprite_.set_width(2.0f);
	camera_image_sprite_.set_height(2.0f * verticalScaleFactor);
	camera_image_sprite_.set_texture(&camera_feed_tex_);
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
				if (!bullet_.GetMoving())
				{
					bullet_.position_.set_y(bullet_.position_.y() + (sensitivity * delta_time));
				}
			}
			// Move left
			if (keyboard->IsKeyDown(gef::Keyboard::KC_A))
			{
				temp_pos_.set_x(temp_pos_.x() - (sensitivity * delta_time));
				if (!bullet_.GetMoving())
				{
					bullet_.position_.set_x(bullet_.position_.x() - (sensitivity * delta_time));
				}
			}
			// Move down
			if (keyboard->IsKeyDown(gef::Keyboard::KC_S))
			{
				temp_pos_.set_y(temp_pos_.y() + (sensitivity * delta_time));
				if (!bullet_.GetMoving())
				{
					bullet_.position_.set_y(bullet_.position_.y() - (sensitivity * delta_time));
				}
			}
			// Move right
			if (keyboard->IsKeyDown(gef::Keyboard::KC_D))
			{
				temp_pos_.set_x(temp_pos_.x() + (sensitivity * delta_time));
				if (!bullet_.GetMoving())
				{
					bullet_.position_.set_x(bullet_.position_.x() + (sensitivity * delta_time));
				}
			}
			// Set cursor to new position based on input
			cursor_.set_position(temp_pos_);

			// Shoot
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE) && !bullet_.GetMoving())
			{
				bullet_.SetActive(true);
				bullet_.position_ = cursor_.position();
				bullet_.velocity_ = gef::Vector4(0.0f, 0.0f, -10.0f);
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