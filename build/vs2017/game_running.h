#pragma once
#include "game_state.h"

#include <system/application.h>
#include <graphics/sprite.h>
#include <maths/vector2.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>
#include <vector>
#include <graphics/mesh_instance.h>
#include "primitive_builder.h"
#include "game_object.h"
#include "load_texture.h"
#include "bullet.h"
#include "duck.h"

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
	void CleanUp();

	gef::Mesh* GetFirstMesh(gef::Scene* scene);
	void ReadSceneAndAssignFirstMesh(const char* filename, gef::Scene** scene, gef::Mesh** mesh);
	bool IsColliding_SphereToSphere(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2);
	bool IsColliding_AABBToAABB(const gef::MeshInstance& meshInstance1, const gef::MeshInstance& meshInstance2);
private:
	void InitFont();
	void DrawHUD();
	void Input(float delta_time);
	void UpdateDucks(float delta_time);
	void SetupLights();
	void SetupCamera();

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

	// Camera
	gef::Vector4 camera_eye_;
	gef::Vector4 camera_lookat_;
	gef::Vector4 camera_up_;
	float camera_fov_;
	float near_plane_;
	float far_plane_;

	// Game objects
	Bullet bullet_;
	std::vector<Duck> ducks_;
	GameObject grass_;

	// UI
	gef::Sprite cursor_;
	gef::Texture* crosshair_;

	// Wave management
	bool next_wave = false;
	float wave_timer = 0.0f;
	float wave_delay = 5.0f;
	int max_ducks = 5;
	int active_duck_count = 0;
	int wave_count = 0;
	void SpawnWave();

	// Score keeping
	int score = 0;

	void CastRayFromCamera();

	// AR
	/*gef::Matrix44 camera_feed_matrix_;
	gef::Matrix44 projection_3d_;
	gef::Sprite camera_image_sprite_;
	gef::TextureVita camera_feed_tex_;
	float cameraAspectRatio;
	float displayAspectRatio;
	float verticalScaleFactor;*/
};

