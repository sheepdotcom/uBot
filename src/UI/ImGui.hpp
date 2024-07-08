#pragma once

#include <imgui-cocos.hpp>
#include "../bot.hpp"

class UwUGui {
public:
	//These are just so ImGui has something to toggle, they aren't the actual value, those are stored in bot.hpp
	bool recording = false;
	bool playing = false;
	int nameCurrent = 0;
	BotFileError error;

	static UwUGui* get();
	void setup();
};