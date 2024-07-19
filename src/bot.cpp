#include "bot.hpp"
#include "../thirdparty/json.hpp"

using namespace geode::prelude;

//Some code is based off xdBot
//xdBot macro file format sucks
//I will make sure mine is better

int uwuBot::getCurrentFrame() {
	return (PlayLayer::get() != nullptr) ? PlayLayer::get()->m_gameState.m_currentProgress : -1; //This is real? Crash fix?
}

void uwuBot::recordInput(bool isPlayer1, int button, int frame, bool holding, playerData playerData) {
	catgirl->m_macroData.push_back({isPlayer1, button, frame, holding, playerData});
}

BotFileError uwuBot::saveMacro(std::string name) {
	if (name.empty()) return BotFileError::EmptyFileName;

	std::string saveLoc = Mod::get()->getSaveDir().string();
	if (!std::filesystem::exists(saveLoc + "/macros")) {
		std::filesystem::create_directory(saveLoc + "/macros");
	}
	saveLoc = saveLoc + "/macros/" + name + ".uwu"; //We love UwU files hehe

	//Similar format to gdr but we store less (since we can infer details when i add the export as different file type feature)
	nlohmann::json json;

	json["bot"] = {
		{"name", "uwuBot"},
		{"version", "1"}
	};
	json["duration"] = uwuBot::catgirl->m_macroData.back().frame;
	json["gameVersion"] = uwuBot::catgirl->m_gdVersion; //Lets use a string instead :3
	json["seed"] = 0; //Add later
	json["inputs"] = nlohmann::json::array();

	for (const auto& frame : uwuBot::catgirl->m_macroData) {
		nlohmann::json input;

		input["frame"] = frame.frame;
		input["btn"] = frame.button;
		input["down"] = frame.holding;
		input["isP1"] = frame.isPlayer1;

		input["pos_x"] = frame.pData.xPos;
		input["pos_y"] = frame.pData.yPos;

		json["inputs"].push_back(input);
	}

	//Do file operations later so it is open the least we can
	std::ofstream file(saveLoc, std::ios::binary);

	if (file.is_open()) {
		std::vector<std::uint8_t> msg_json = nlohmann::json::to_msgpack(json); //geode::ByteVector
		file.write(reinterpret_cast<const char*>(msg_json.data()), msg_json.size()); //I might use messagepack or I might create my own format :3
		
		file.close();
	}
	else {
		return BotFileError::UnableToOpenFile;
	}
	return BotFileError::Success;
}

BotFileError uwuBot::loadMacro(std::string name) {
	if (name.empty()) return BotFileError::EmptyFileName;

	std::string saveLoc = Mod::get()->getSaveDir().string();
	saveLoc = saveLoc + "/macros/" + name + ".uwu"; //We love UwU files hehe
	
	if (!std::filesystem::exists(saveLoc)) {
		return BotFileError::InvalidFileName;
	}

	std::ifstream file(saveLoc, std::ios::binary);

	if (!file.is_open()) {
		return BotFileError::UnableToOpenFile;
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	
	geode::ByteVector data(fileSize);
	file.read(reinterpret_cast<char*>(data.data()), fileSize);
	file.close();
	nlohmann::json json;

	json = nlohmann::json::from_msgpack(data, true, false);
	if (json.is_discarded()) {
		return BotFileError::UnableToOpenFile;
	}

	if (!uwuBot::catgirl->m_macroData.empty()) {
		uwuBot::catgirl->m_macroData.clear();
	}

	nlohmann::json inputs = json["inputs"];
	for (auto& input : inputs) {
		auto frame = input["frame"].get<int>();
		auto button = input["btn"].get<int>();
		auto holding = input["down"].get<bool>();
		auto isPlayer1 = input["isP1"].get<bool>();

		auto xPos = input["pos_x"].get<float>();
		auto yPos = input["pos_y"].get<float>();

		uwuBot::catgirl->recordInput(isPlayer1, button, frame, holding, {xPos, yPos});
	}

	return BotFileError::Success;
}

void uwuBot::clearState() {
	uwuBot::catgirl->m_state = state::off;
	uwuBot::catgirl->updateLabels();
}

void uwuBot::updateLabels() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	if (uwuBot::catgirl->m_state == state::off) {
		if (uwuBot::catgirl->frameLabel != nullptr) {
			uwuBot::catgirl->frameLabel->removeFromParent();
			uwuBot::catgirl->frameLabel = nullptr;
		}
		if (uwuBot::catgirl->stateLabel != nullptr) {
			uwuBot::catgirl->stateLabel->removeFromParent();
			uwuBot::catgirl->stateLabel = nullptr;
		}
	}
	else {
		if (uwuBot::catgirl->frameLabel == nullptr && PlayLayer::get()) {
			uwuBot::catgirl->frameLabel = CCLabelBMFont::create("Frame: 0", "bigFont.fnt");
			uwuBot::catgirl->frameLabel->setAnchorPoint(ccp(0.f, 1.f));
			uwuBot::catgirl->frameLabel->setPosition(ccp(0.f, winSize.height));
			uwuBot::catgirl->frameLabel->setID("frame-label");
			uwuBot::catgirl->frameLabel->setScale(0.5f);
			uwuBot::catgirl->frameLabel->setZOrder(128);

			PlayLayer::get()->m_uiLayer->addChild(uwuBot::catgirl->frameLabel);
		}

		if (uwuBot::catgirl->stateLabel == nullptr && PlayLayer::get()) {
			auto pos = ccp(0.f, winSize.height);
			if (uwuBot::catgirl->frameLabel != nullptr) {
				pos.y -= uwuBot::catgirl->frameLabel->getScaledContentHeight(); //Finally no warning about NULL stuff
			}
			uwuBot::catgirl->stateLabel = CCLabelBMFont::create("Off", "bigFont.fnt");
			uwuBot::catgirl->stateLabel->setAnchorPoint(ccp(0.f, 1.f));
			uwuBot::catgirl->stateLabel->setPosition(pos);
			uwuBot::catgirl->stateLabel->setID("state-label");
			uwuBot::catgirl->stateLabel->setScale(0.5f);
			uwuBot::catgirl->stateLabel->setZOrder(128);

			PlayLayer::get()->m_uiLayer->addChild(uwuBot::catgirl->stateLabel);
		}
	}

	if (uwuBot::catgirl->m_state == state::recording) {
		if (stateLabel != nullptr) {
			stateLabel->setString("Recording");
		}
	}
	if (uwuBot::catgirl->m_state == state::playing) {
		if (stateLabel != nullptr) {
			stateLabel->setString("Playing");
		}
	}
}