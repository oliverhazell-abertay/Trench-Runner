#include "menu_object.h"

#define ALL_BUTTONS int button_num = 0; button_num < buttons_.size(); button_num++

void MenuObject::Init()
{
	selected_colour = gef::Colour(1.0f, 1.0f, 1.0f, 1.0f);
	unselected_colour = gef::Colour(1.0f, 0.1f, 0.1f, 0.1f);
}

void MenuObject::CleanUp()
{
	for (ALL_BUTTONS)
	{
		delete buttons_[button_num];
		buttons_[button_num] = NULL;
	}
}

void MenuObject::Update(float delta_time)
{
	for (ALL_BUTTONS)
	{
		if (button_num == currentButton_)
			buttons_[button_num]->button_sprite.set_colour(selected_colour.GetRGBA());
		else
			buttons_[button_num]->button_sprite.set_colour(unselected_colour.GetRGBA());
	}
}

void MenuObject::NextButton()
{
	currentButton_++;
	// Check for currentButton_ going off end of vector, put to start
	if (currentButton_ >= buttons_.size())
		currentButton_ = 0;
}

void MenuObject::PrevButton()
{
	currentButton_--;
	// Check for currentButton_ going off end of vector, put to end
	if (currentButton_ < 0)
		currentButton_ = buttons_.size() - 1;
}
