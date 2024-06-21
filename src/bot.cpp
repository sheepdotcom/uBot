#include "bot.hpp"

using namespace geode::prelude;

//Some code is based off xdBot
//xdBot macro file format sucks
//I will make sure mine is better

uwuBot* uwuBot::get() {
	return catgirl;
}

int uwuBot::getCurrentFrame() {
	return (PlayLayer::get() != nullptr) ? PlayLayer::get()->m_gameState.m_currentProgress : -1; //This is real? Crash fix?
}

void uwuBot::recordInput(bool isPlayer1, int button, int frame, bool holding, playerData playerData) {
	catgirl->m_macroData.push_back({ isPlayer1, button, frame, holding, playerData });
}

BotFileError uwuBot::saveMacro(std::string name) {
	if (name.empty()) return BotFileError::EmptyFileName;

	std::string saveLoc = Mod::get()->getSaveDir().string();
	saveLoc = saveLoc + "/" + name + ".uwu"; //We love UwU files hehe

	std::ofstream file(saveLoc);

	if (file.is_open()) {
		//Not final method
		for (auto& action : catgirl->m_macroData) {
			file << action.frame << "|" << action.holding << "|" << action.button << "|" << action.isPlayer1 << "|" << action.pData.xPos << "|" << action.pData.yPos << "\n";
		}
		file.close();
	}
	else {
		return BotFileError::UnableToOpenFile;
	}
	return BotFileError::Success;
}

void uwuBot::clearState() {
	catgirl->m_state = state::off;
}

void uwuBot::reset() {
	catgirl->m_currentAction = 0;
	for (size_t player = 0; player < 2; player++) {
		auto p = ((player == 0) ? GJBaseGameLayer::get()->m_player1 : GJBaseGameLayer::get()->m_player2);
		p->releaseAllButtons();
	}
}