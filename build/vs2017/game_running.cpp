#include "game_running.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <graphics/font.h>
#include <maths/vector2.h>
#include <input/input_manager.h>
#include <input/sony_controller_input_manager.h>
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
#define ALL_PILLARS int pillar_num = 0; pillar_num < pillars_.size(); pillar_num++

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

	// Player object init
	player_.player_object.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	player_.player_object.position_ = camera_eye_;
	player_.player_object.scale_ = gef::Vector4(10.0f, 10.0f, 10.0f);

	// Enemy Init
	enemy_ = new Enemy(primitive_builder_, gef::Vector4(-10.0f, -15.0f, 530.0f));
	enemy_->ScaleObjects(gef::Vector4(2.0f, 2.0f, 2.0f));
	enemy_->StartMoving(gef::Vector4(5.0f, 5.0f, 300.0f), 2.0f);

	// Init floor
	floor_material.set_texture(CreateTextureFromPNG("walls_big.png", *platform));
	floor_.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	gef::Vector4 floor_translation_(0.0f, -100.0f, 0.0f);
	floor_.position_ = floor_translation_;
	floor_.scale_ = gef::Vector4(wall_size_, 1.0f, wall_size_);
	floor_.velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
	// Init walls
	gef::Vector4 wall_translation;
	// Left wall
	left_wall_.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	wall_translation = gef::Vector4(-200.0f, 0.0f, 0.0f);
	left_wall_.position_ = wall_translation;
	left_wall_.scale_ = gef::Vector4(1.0f, 200.0f, wall_size_);
	left_wall_.velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
	// Right walls
	right_wall_.set_mesh(primitive_builder_->GetDefaultCubeMesh());
	wall_translation = gef::Vector4(200.0f, 0.0f, 0.0f);
	right_wall_.position_ = wall_translation;
	right_wall_.scale_ = gef::Vector4(1.0f, 200.0f, wall_size_);
	right_wall_.velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);

	// Pillars init
	// Pillar 1
	GameObject* tempPillar;
	tempPillar = new GameObject;
	tempPillar->set_mesh(primitive_builder_->GetDefaultCubeMesh());
	tempPillar->position_ = gef::Vector4(0.0f, 0.0f, -500.0f);
	tempPillar->scale_ = gef::Vector4(400.0f, 66.7f, 50.0f);
	tempPillar->velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
	pillars_.push_back(tempPillar);
	// Pillar 2
	tempPillar = new GameObject;
	tempPillar->set_mesh(primitive_builder_->GetDefaultCubeMesh());
	tempPillar->position_ = gef::Vector4(0.0f, 0.0f, -1000.0f);
	tempPillar->scale_ = gef::Vector4(400.0f, 66.7f, 50.0f);
	tempPillar->velocity_ = gef::Vector4(0.0f, 0.0f, scroll_speed_);
	pillars_.push_back(tempPillar);

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
		SetupCamera();
	}
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

	for (ALL_PILLARS)
	{
		delete pillars_[pillar_num];
		pillars_[pillar_num] = NULL;
	}

	delete enemy_;
	enemy_ = NULL;
}

void GameRunning::Update(float delta_time)
{
	// Score
	score += delta_time * 5.5f;
	// Input
	Input(delta_time);
	// Update Player
	player_.Update(delta_time);

	// collision detection with walls
	if (IsColliding_AABBToAABB(player_.player_object, left_wall_)
			|| IsColliding_AABBToAABB(player_.player_object, right_wall_)
				|| IsColliding_AABBToAABB(player_.player_object, floor_))
	{
		signal_to_change = GAMEOVER;
	}
	// Upper bounds check
	if (player_.player_object.position_.y() > 100.0f)
		player_.player_object.position_.set_y(100.0f);
	// Update lasers
	UpdateLasers(delta_time);
	// Walls update
	UpdateWalls(delta_time);
	// Pillar update
	UpdatePillars(delta_time);
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
		// Draw walls
		renderer_3d_->set_override_material(&floor_material);
		renderer_3d_->DrawMesh(floor_);
		renderer_3d_->DrawMesh(left_wall_);
		renderer_3d_->DrawMesh(right_wall_);
		//Draw pillars
		// If pillar 1 is closer, draw pillar 2 first
		/*if (pillars_[0]->position_.z() > pillars_[1]->position_.z())
		{
			renderer_3d_->DrawMesh(*pillars_[1]);
			renderer_3d_->DrawMesh(*pillars_[0]);
		}
		else
		{
			renderer_3d_->DrawMesh(*pillars_[0]);
			renderer_3d_->DrawMesh(*pillars_[1]);
		}*/
		// Draw Enemy
		if (enemy_)
		{
			renderer_3d_->set_override_material(&enemy_->material);
			renderer_3d_->DrawMesh(enemy_->cockpit_);
			renderer_3d_->DrawMesh(enemy_->left_wing_);
			renderer_3d_->DrawMesh(enemy_->right_wing_);
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
	// Draw Score
	if (font_)
	{
		font_->RenderText(sprite_renderer_, gef::Vector4(0.0f, 0.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "Score: %.0f", score);
	}
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

	// Initialise play position at camera
	player_.player_object.position_ = camera_eye_;
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

		// Reset cursor
		cursor_.set_position(gef::Vector4(SCREEN_CENTRE_X, SCREEN_CENTRE_Y, 0.0f));

		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			int sensitivity = 250;
			if (keyboard->IsKeyDown(gef::Keyboard::KC_LSHIFT))
				sensitivity = 100;
			// Shoot
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_SPACE)) //&& !bullet_.GetMoving())
			{
				Fire();
			}
			// Pause
			if (keyboard->IsKeyPressed(gef::Keyboard::KC_P))
			{
				signal_to_change = PAUSE;
			}
			// Movement
			Movement(keyboard);
		}
	}
}

void GameRunning::Fire()
{
	// Left Laser
	Bullet* tempLeftLaser;
	tempLeftLaser = new Bullet;
	tempLeftLaser->SetActive(true);
	tempLeftLaser->set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(0.1f, 0.1f, 10.0f)));
	tempLeftLaser->position_ = camera_eye_;
	tempLeftLaser->position_.set_x(camera_eye_.x() - 1.0f);
	tempLeftLaser->position_.set_y(camera_eye_.y() - 1.0f);
	tempLeftLaser->velocity_ = gef::Vector4(0.0f, 0.0f, (float)shootSpeed * -1);
	lasers_.push_back(tempLeftLaser);

	// Right Laser
	Bullet* tempRightLaser;
	tempRightLaser = new Bullet;
	tempRightLaser->SetActive(true);
	tempRightLaser->set_mesh(primitive_builder_->CreateBoxMesh(gef::Vector4(0.1f, 0.1f, 10.0f)));
	tempRightLaser->position_ = camera_eye_;
	tempRightLaser->position_.set_x(camera_eye_.x() + 1.0f);
	tempRightLaser->position_.set_y(camera_eye_.y() - 1.0f);
	tempRightLaser->velocity_ = gef::Vector4(0.0f, 0.0f, (float)shootSpeed * -1);
	lasers_.push_back(tempRightLaser);
}

void GameRunning::Movement(gef::Keyboard* keyboard_)
{
	// Set default rotation
	gef::Vector4 temp_rot_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	// Get and store player current velocity
	gef::Vector4 temp_velocity = player_.player_object.velocity_;
	// Move up
	if (keyboard_->IsKeyDown(gef::Keyboard::KC_W))
	{
		temp_velocity.set_y(temp_velocity.y() + moveSpeed);
	}
	// Move down
	if (keyboard_->IsKeyDown(gef::Keyboard::KC_S))
	{
		temp_velocity.set_y(temp_velocity.y() - moveSpeed);
	}
	// Move left
	if (keyboard_->IsKeyDown(gef::Keyboard::KC_A))
	{
		temp_velocity.set_x(temp_velocity.x() - moveSpeed);
		temp_rot_.set_x(turn_tilt * -1);
	}
	// Move right
	if (keyboard_->IsKeyDown(gef::Keyboard::KC_D))
	{
		temp_velocity.set_x(temp_velocity.x() + moveSpeed);
		temp_rot_.set_x(turn_tilt);
	}
	// Work out direction player is headed
	int horDirection = 0;
	int vertDirection = 0;
	horDirection = (temp_velocity.x() > 0) ? -1 : 1;
	vertDirection = (temp_velocity.y() > 0) ? -1 : 1;
	// Damping
	if(player_.player_object.velocity_.x() != 0.0f)	// Only dampen if player is moving
		temp_velocity.set_x(temp_velocity.x() + (horDamping * horDirection));
	if(player_.player_object.velocity_.y() != 0.0f) // Only dampen if player is moving
		temp_velocity.set_y(temp_velocity.y() + (vertDamping * vertDirection));

	// Update player velocity based on input
	player_.player_object.velocity_ = temp_velocity;

	// Set camera to player position and rotation based on input
	camera_eye_ = player_.player_object.position_;
	camera_up_ = temp_rot_;
	// Keep camera looking straight forward
	camera_lookat_ = player_.player_object.position_;
	camera_lookat_.set_z(camera_lookat_.z() - 1.0f);

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
		// If laser is active
		if (lasers_[laser_num]->GetActive())
		{
			// If laser hit pillar
			if (IsColliding_AABBToAABB(*lasers_[laser_num], *pillars_[0])
				|| IsColliding_AABBToAABB(*lasers_[laser_num], *pillars_[1]))
			{
				// Delete laser
				lasers_[laser_num]->SetActive(false);
			}
			// If laser hit enemy object
			if (IsColliding_AABBToAABB(*lasers_[laser_num], enemy_->cockpit_))
			{
				// Delete laser
				lasers_[laser_num]->SetActive(false);
				// Mark enemy for deletion
				enemy_->material = primitive_builder_->red_material();
				enemy_->MarkForDeletion(true);
			}
		}
	}
	// "Delete" enemy by moving enemy behind camera
	if (enemy_->ToBeDeleted())
	{
		enemy_->position = camera_eye_;
		enemy_->position.set_z(enemy_->position.z() + 100.0f);
		enemy_->alive = false;
		enemy_->MarkForDeletion(false);
		SpawnEnemy(gef::Vector4(-10.0f, -15.0f, 530.0f), gef::Vector4(5.0f, 5.0f, 300.0f));
	}
}

void GameRunning::UpdateWalls(float delta_time)
{
	// Left walls
	left_wall_.Update(delta_time);
	// If wall is off camera, put to the start of the treadmill
	if (left_wall_.position_.z() > 500.0f)
		left_wall_.position_.set_z(0.0f);

	// Right walls
	right_wall_.Update(delta_time);
	// If wall is off camera, put to the start of the treadmill
	if (right_wall_.position_.z() > 500.0f)
		right_wall_.position_.set_z(0.0f);

	// Floor
	floor_.Update(delta_time);
	// If wall is off camera, put to the start of the treadmill
	if (floor_.position_.z() > 500.0f)
		floor_.position_.set_z(0.0f);
}

void GameRunning::UpdatePillars(float delta_time)
{
	for (ALL_PILLARS)
	{
		// Update pillar
		pillars_[pillar_num]->Update(delta_time);
		// Reset pillar if it has passed the player
		if (pillars_[pillar_num]->position_.z() > 550.0f)
		{
			SpawnPillar(pillars_[pillar_num]);
		}
	}
}

void GameRunning::SpawnPillar(GameObject* nextPillar)
{
	// Random y
	nextPillar->position_.set_y((float)(rand() % 200 - 100));
	// Spawn pillar far down track in front of player
	nextPillar->position_.set_z((float)(rand() % 500 - 1500));
}

void GameRunning::SpawnEnemy(gef::Vector4 initPos, gef::Vector4 initTarget)
{
	enemy_->material = primitive_builder_->blue_material();
	enemy_->position = gef::Vector4(-10.0f, -15.0f, 530.0f);
	enemy_->StartMoving(gef::Vector4(5.0f, 5.0f, 300.0f), 2.0f);
	enemy_->alive = true;
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

	bullet->position_ = startPoint;
	bullet->velocity_ = direction * shootSpeed;
}