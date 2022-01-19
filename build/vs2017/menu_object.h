#pragma once
#include "ui_button.h"
#include <graphics/material.h>
#include <graphics/colour.h>
#include<vector>

class MenuObject
{
public:
	void Init();
	void CleanUp();
	void Update(float delta_time);
	void NextButton();
	void PrevButton();
	
	int currentButton_ = 0;
	int lastCurrentButton_ = 0;
	std::vector<UIButton*> buttons_;

	gef::Colour selected_colour;
	gef::Colour unselected_colour;
};

