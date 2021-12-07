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
#define ALL_LASERS int laser_num = 0; laser_num < lasers_.size(); laser_num++
#define ALL_WALLS int wall_num = 0; wall_num < 5; wall_num++

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

	// Enemy Init
	enemy_ = new Enemy;
	ReadSceneAndAssignFirstMesh("ship.scn", &model_scene_, &mesh_);
	enemy_->set_mesh(mesh_);
	enemy_->position_ = gef::Vector4(-10.0f, -15.0f, 530.0f);
	enemy_->rotation_ = gef::Vector4(4.71f, 0.0f, 0.0f);
	enemy_->material = primitive_builder_->blue_material();
	enemy_->StartMoving(gef::Vector4(5.0f, 5.0f, 480.0f), 2.0f);

	// Init floor
	floor_material.set_texture(CreateTextureFromPNG("Wall.png", *platform));
	for (ALL_WALLS)
	{
		GameObject* tempFloor;
		tempFloor = new GameObject;
		tempFloor->set_mesh(primitive_builder_->GetDefaultCubeMesh());
		gef::Vector4 floor_translation_(0.0f, -100.0f, wall_size_ * wall_num * -1);
		tempFloor->position_ = floor_translation_;
		tempFloor->scale_ = gef::Vector4(wall_size_, 1.0f, wall_size_);
		tempFloor->velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
		floors_.push_back(tempFloor);
	}
	// Init walls
	gef::Vector4 wall_translation;
	// Left wall
	for (ALL_WALLS)
	{
		GameObject* tempWall;
		tempWall = new GameObject;
		tempWall->set_mesh(primitive_builder_->GetDefaultCubeMesh());
		wall_translation = gef::Vector4(-200.0f, 0.0f, wall_size_ * wall_num * -1);
		tempWall->position_ = wall_translation;
		tempWall->scale_ = gef::Vector4(1.0f, 200.0f, wall_size_);
		tempWall->velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
		left_walls_.push_back(tempWall);
	}
	// Right walls
	for (ALL_WALLS)
	{
		GameObject* tempWall;
		tempWall = new GameObject;
		tempWall->set_mesh(primitive_builder_->GetDefaultCubeMesh());
		wall_translation = gef::Vector4(200.0f, 0.0f, wall_size_ * wall_num * -1);
		tempWall->position_ = wall_translation;
		tempWall->scale_ = gef::Vector4(1.0f, 200.0f, wall_size_);
		tempWall->velocity_ = gef::Vector4(0.0f, 0.0f, 300.0f);
		right_walls_.push_back(tempWall);
	}


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

	for (ALL_LASERS)
	{
		delete lasers_[laser_num];
		lasers_[laser_num] = NULL;
	}

	delete enemy_;
	enemy_ = NULL;

	for (ALL_WALLS)
	{
		delete left_walls_[wall_num];
		left_walls_[wall_num] = NULL;
		delete right_walls_[wall_num];
		right_walls_[wall_num] = NULL;
		delete floors_[wall_num];
		floors_[wall_num] = NULL;
	}
}

void GameRunning::Update(float delta_time)
{
	// Input
	Input(delta_time);
	// Update lasers
	UpdateLasers(delta_time);
	// Walls update
	UpdateWalls(delta_time);
	// Enemy update
	if (enemy_)
		enemy_->Update(delta_time);
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
		renderer_3d_->set_override_material(&floor_material);
		for (ALL_WALLS)
		{
			renderer_3d_->DrawMesh(*floors_[wall_num]);
		}
		// Draw walls
		renderer_3d_->set_override_material(&floor_material);
		for (ALL_WALLS)
		{
			renderer_3d_->DrawMesh(*left_walls_[wall_num]);
			renderer_3d_->DrawMesh(*right_walls_[wall_num]);
		}
		// Draw Enemy
		if (enemy_)
		{
			renderer_3d_->set_override_material(&enemy_->material);
			renderer_3d_->DrawMesh(*enemy_);
		}
		// Draw lasers
		for (int laser_num = 0; laser_num < lasers_.size(); laser_num++)
		{
			if (lasers_[laser_num]->GetActive())
			{
				renderer_3d_->set_override_material(&primitive_builder_->red_material());
				renderer_3d_->DrawMesh(*lasers_.at(laser_num));
			}
		}
	// Reset override material
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
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 0.7f, 1.0f));
	default_point_light.set_position(gef::Vector4(0.0f, -500.0f, 600.0f));

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

void GameRunning::Input(float delta_time)
{
	// Input
	if (input_manager_)
	{
		input_manager_->Update();

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
			}
			// Move left
			if (keyboard->IsKeyDown(gef::Keyboard::KC_A))
			{
				temp_pos_.set_x(temp_pos_.x() - (sensitivity * delta_time));
			}
			// Move down
			if (keyboard->IsKeyDown(gef::Keyboard::KC_S))
			{
				temp_pos_.set_y(temp_pos_.y() + (sensitivity * delta_time));
			}
			// Move right
			if (keyboard->IsKeyDown(gef::Keyboard::KC_D))
			{
				temp_pos_.set_x(temp_pos_.x() + (sensitivity * delta_time));
			}
			// Set cursor to new position based on input
			cursor_.set_position(temp_pos_);

			// Shoot
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) //&& !bullet_.GetMoving())
			{
				Bullet* tempLaser;
				tempLaser = new Bullet;
				tempLaser->SetActive(true);
				tempLaser->set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(0.1f, 0.1f, 1.0f)));
				CastRayFromCamera(tempLaser);
				lasers_.push_back(tempLaser);
			}
			// Pause
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_P))
			{
				signal_to_change = PAUSE;
			}
		}
	}
}

void GameRunning::UpdateLasers(float delta_time)
{
	// Update lasers
	for (ALL_LASERS)
	{
		if (lasers_[laser_num]->GetActive())
		{
			lasers_.at(laser_num)->Update(delta_time);
		}
	}
	// Check for collisions
	for (ALL_LASERS)
	{
		if (IsColliding_AABBToAABB(*lasers_[laser_num], *enemy_))
		{
			//enemy_.material = primitive_builder_->red_material();
			enemy_->MarkForDeletion(true);
		}
	}
	if (enemy_->ToBeDeleted())
	{
		enemy_->position_ = camera_eye_;
		enemy_->position_.set_z(enemy_->position_.z() + 100.0f);
	}
}

void GameRunning::UpdateWalls(float delta_time)
{
	// Left walls
	for (ALL_WALLS)
	{
		left_walls_[wall_num]->Update(delta_time);
		// If wall is off camera, put to the start of the treadmill
		if (left_walls_[wall_num]->position_.z() > 1000.0f)
			left_walls_[wall_num]->position_.set_z((left_walls_.size() - 1) * (wall_size_ * -1));
	}
	// Right walls
	for (ALL_WALLS)
	{
		right_walls_[wall_num]->Update(delta_time);
		// If wall is off camera, put to the start of the treadmill
		if (right_walls_[wall_num]->position_.z() > 1000.0f)
			right_walls_[wall_num]->position_.set_z((right_walls_.size() - 1) * (wall_size_ * -1));
	}
	// Floor
	for (ALL_WALLS)
	{
		floors_[wall_num]->Update(delta_time);
		// If wall is off camera, put to the start of the treadmill
		if (floors_[wall_num]->position_.z() > 1000.0f)
			floors_[wall_num]->position_.set_z((floors_.size() - 1) * (wall_size_ * -1));
	}
}

// https://antongerdelan.net/opengl/raycasting.html
void GameRunning::CastRayFromCamera(Bullet* bullet)
{
	gef::Vector4 startPoint;
	gef::Vector4 direction;

	gef::Vector2 ndc;

	float hw = platform_->width() * 0.5f;
	float hh = platform_->height() * 0.5f;

	ndc.x = (static_cast<float>(cursor_.position().x()) - hw) / hw;
	ndc.y = (hh - static_cast<float>(cursor_.position().y() - 10.0f)) / hh;

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

	startPoint.set_y(startPoint.y() - 0.5f);
	bullet->position_ = startPoint;
	bullet->velocity_ = direction * shootSpeed;
}