#pragma once
#include <graphics/sprite.h>
#include <graphics/material.h>

enum ButtonType { MAINMENU, STARTGAME, OPTIONS, LEFT, RIGHT, BACK };

class UIButton
{
public:
	UIButton(gef::Sprite sprite);
	gef::Sprite button_sprite;
};

