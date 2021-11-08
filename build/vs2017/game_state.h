#pragma once
enum Type { MENU, GAMERUNNING, PAUSE, GAMEOVER, EMPTY};

class GameState
{
public:
	GameState();
	~GameState();
	Type GetType() { return type_; }
	// Overrides
	virtual void OnEntry(Type prev_game_state) = 0;
	virtual void OnExit(Type next_game_state) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	Type signal_to_change = EMPTY;
protected:
	Type type_ = EMPTY;
};

