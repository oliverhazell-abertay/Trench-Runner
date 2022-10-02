#pragma once
#include "game_state.h"

#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>
#include <vector>
#include <graphics/mesh_instance.h>
#include <input/keyboard.h>
#include "primitive_builder.h"
#include "game_object.h"
#include "load_texture.h"
#include "bullet.h"
#include "player.h"
#include "enemy.h"


//// Vita AR includes
//#include <camera.h>
//#include <gxm.h>
//#include <motion.h>
//#include <libdbg.h>
//#include <libsmart.h>

namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Sprite;
	class Font;
	class Renderer3D;
	class Mesh;
	class Scene;
	class InputManager;
	class RenderTarget;
	class TextureVita;
}

class GameRunning : public GameState
{
public:
	GameRunning(gef::Platform* platform, gef::SpriteRenderer* sprite_rend_, gef::Renderer3D* rend_3d_, gef::InputManager* in_);
	~GameRunning();
	virtual void OnEntry(Type prev_game_state) override;
	virtual void OnExit(Type next_game_state) override;
	virtual void Update(float delta_time) override;
	virtual void Render() override;
	void Init();
	void CleanUp();

	gef::Mesh* GetFirstMesh(gef::Scene* scene);
	void ReadSceneAndAssignFirstMesh(const char* filename, gef::Scene** scene, gef::Mesh** mesh);
	bool IsColliding_SphereToSphere(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2);
	bool IsColliding_AABBToAABB(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2);
private:
	void InitFont();
	void DrawHUD();
	void Input(float delta_time);
	void SetupLights();
	void SetupCamera();
	void UpdateLasers(float delta_time);
	void UpdateWalls(float delta_time);
	void UpdatePillars(float delta_time);
	void SpawnPillar(GameObject* nextPillar);
	void SpawnEnemy(gef::Vector4 initPos, gef::Vector4 initTarget);

	gef::Platform* platform_;
	gef::SpriteRenderer* sprite_renderer_;
	gef::Renderer3D* renderer_3d_;
	gef::InputManager* input_manager_;
	gef::Font* font_;
	PrimitiveBuilder* primitive_builder_;
	gef::Scene* model_scene_;
	gef::Mesh* mesh_; 

	// Game logic
	bool isColliding = false;
	void Fire();
	void Movement(gef::Keyboard* keyboard_);

	// Camera
	gef::Vector4 camera_eye_;
	gef::Vector4 camera_lookat_;
	gef::Vector4 camera_up_;
	float camera_fov_;
	float near_plane_;
	float far_plane_;

	// Game objects
	std::vector<Bullet*> lasers_;
	Player* player_;
	GameObject floor_;
	gef::Material floor_material;
	GameObject left_wall_;
	GameObject right_wall_;
	gef::Material wall_material;
	std::vector<GameObject*> pillars_;
	gef::Material pillar_material;
	int closest_pillar_ = 0;
	Enemy* enemy_;
	GameObject sky_;
	gef::Material skybox_black;
	GameObject fog_;
	gef::Material fog_colour;

	// UI
	gef::Sprite cursor_;
	gef::Sprite hud_;
	gef::Texture* crosshair_;
	gef::Texture* hud_tex_;

	// Treadmill management
	const float wall_size_ = 5000.0f;
	float scroll_speed_ = 220.0f;
	float enemy_spawn_timer = 0.0f;
	float enemy_spawn_max = 3.0f;

	// Game over
	void GameOverTransition(float delta_time);
	float gameOverTimer = 0.0f;
	float gameOverTimerMax = 0.5f;

	// Movement
	float moveSpeed = 5.0f;
	float horDamping = 2.5f;
	float vertDamping = 2.5f;

	// Shooting
	int shootSpeed = 1000;
	float turn_tilt = 0.0025f;
	void CastRayFromCamera(Bullet* bullet);

	// Screen swipe
	bool ScreenSwipe(float delta_time, int dir);
	gef::Sprite screen_swipe_sprite;
	bool transistion = false;
	bool swipeMoving = false;
	bool gameStarted = false;

	// AR
	/*gef::Matrix44 camera_feed_matrix_;
	gef::Matrix44 projection_3d_;
	gef::Sprite camera_image_sprite_;
	gef::TextureVita camera_feed_tex_;
	float cameraAspectRatio;
	float displayAspectRatio;
	float verticalScaleFactor;*/
};

