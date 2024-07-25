#include "../thirdparty/json.hpp"
#include "bot.hpp"

using namespace geode::prelude;

//Some code is based off xdBot
//xdBot macro file format sucks
//I will make sure mine is better

int uwuBot::getCurrentFrame() {
	return (GJBaseGameLayer::get() != nullptr) ? GJBaseGameLayer::get()->m_gameState.m_currentProgress : -1; //This is real? Crash fix?
}

void uwuBot::recordInput(bool isPlayer1, int button, int frame, bool holding, playerData playerData) {
	uwuBot::catgirl->m_macroData.push_back({isPlayer1, button, frame, holding, playerData});
}

void uwuBot::clearInputsAfterFrame(int frame) {
	GJBaseGameLayer* catgirlGame = GJBaseGameLayer::get();

	if (uwuBot::catgirl->m_state == state::recording) {
		if (frame >= 0) {
			int frame = uwuBot::catgirl->getCurrentFrame();
			if (!uwuBot::catgirl->m_macroData.empty()) {
				for (int i = uwuBot::catgirl->m_macroData.size() - 1; i >= 0; i--) { //reverse iterator seems wierd so im using this
					if (uwuBot::catgirl->m_macroData[i].frame >= frame) {
						uwuBot::catgirl->m_macroData.erase(uwuBot::catgirl->m_macroData.begin() + i);
					}
					else break;
				}
				if (uwuBot::catgirl->m_macroData.back().holding) {
					//Add code to fix loading checkpoint where player was holding and hasnt released the button
					auto nya = (catgirlGame->m_gameState.m_isDualMode) ? 2 : 1;
					geode::log::debug("nya {}", nya);
					for (size_t player = 0; player < nya; player++) {
						geode::log::debug("p {} nya {}", player, nya);
						auto p = (player == 0) ? catgirlGame->m_player1 : catgirlGame->m_player2;
						if (p) {
							catgirlGame->handleButton(false, 1, (player == 0));
							if (catgirlGame->m_levelSettings->m_platformerMode) {
								catgirlGame->handleButton(false, 2, (player == 0));
								catgirlGame->handleButton(false, 3, (player == 0));
							}
						}
					}
				}
			}
		}
		else {
			if (!uwuBot::catgirl->m_macroData.empty()) {
				uwuBot::catgirl->m_macroData.clear();
			}
		}
	}
}

void uwuBot::updateInfo(bool posFix, bool yVelFix, bool xVelFix, bool platformer) {
	uwuBot::catgirl->m_infoData.posFix = posFix;
	uwuBot::catgirl->m_infoData.yVelFix = yVelFix;
	uwuBot::catgirl->m_infoData.xVelFix = xVelFix;
	uwuBot::catgirl->m_infoData.platformer = platformer;
}

BotFileError uwuBot::saveMacro(std::string name) {
	if (name.empty()) return BotFileError::EmptyFileName;

	std::string saveLoc = Mod::get()->getSaveDir().string();
	if (!std::filesystem::exists(saveLoc + "/macros")) std::filesystem::create_directory(saveLoc + "/macros");
	saveLoc = saveLoc + "/macros/" + name + ".ubot"; //Goodbye UwU files

	//Similar format to gdr but we store less (since we can infer details when i add the export as different file type feature)
	nlohmann::json json;

	json["bot"] = {
		{"name", "uwuBot"},
		{"version", "2"}
	};
	json["data"]["posFix"] = uwuBot::catgirl->m_infoData.posFix;
	json["data"]["yVelFix"] = uwuBot::catgirl->m_infoData.yVelFix;
	json["data"]["xVelFix"] = uwuBot::catgirl->m_infoData.xVelFix;
	json["data"]["plat"] = uwuBot::catgirl->m_infoData.platformer;
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

		if (uwuBot::catgirl->m_infoData.posFix) {
			input["pos_x"] = frame.pData.xPos;
			input["pos_y"] = frame.pData.yPos;
		}
		if (uwuBot::catgirl->m_infoData.xVelFix && uwuBot::catgirl->m_infoData.platformer) input["vel_x"] = frame.pData.xVel;
		if (uwuBot::catgirl->m_infoData.yVelFix) input["vel_y"] = frame.pData.yVel;

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
	if (!std::filesystem::exists(saveLoc + "/macros")) std::filesystem::create_directory(saveLoc + "/macros");
	saveLoc = saveLoc + "/macros/" + name + ".ubot"; //Goodbye UwU Files

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
	
	//Check if it exists (older versions do not have this)
	if (json.contains("data")) {
		uwuBot::catgirl->m_infoData = {
			json["data"]["posFix"],
			json["data"]["yVelFix"],
			json["data"]["xVelFix"],
			json["data"]["plat"]
		};
	}
	else uwuBot::catgirl->m_infoData = {true, false, false, false}; //Fallback for backwards compatibility (pos fix existed in v1)

	nlohmann::json inputs = json["inputs"];
	for (auto& input : inputs) {
		auto frame = input["frame"].get<int>();
		auto button = input["btn"].get<int>();
		auto holding = input["down"].get<bool>();
		auto isPlayer1 = input["isP1"].get<bool>();

		float xPos = 0.f;
		float yPos = 0.f;
		double xVel = 0.0;
		double yVel = 0.0;
		if (input.contains("pos_x")) xPos = input["pos_x"].get<float>();
		if (input.contains("pos_y")) yPos = input["pos_y"].get<float>();
		if (input.contains("vel_x")) xVel = input["vel_x"].get<double>();
		if (input.contains("vel_y")) yVel = input["vel_y"].get<double>();

		uwuBot::catgirl->recordInput(isPlayer1, button, frame, holding, {xPos, yPos, xVel, yVel});
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

//Practice Bug Fix, using the same method as eclipse menu :3
PlayerSaveObject::PlayerSaveObject(PlayerObject* player) {
	m_unk518 = player->m_unk518;
	m_unk51c = player->m_unk51c;
	m_unk51d = player->m_unk51d;
	m_unk51e = player->m_unk51e;
	m_unk51f = player->m_unk51f;
	m_unk523 = player->m_unk523;
	m_unk568 = player->m_unk568;
	unk_584 = player->unk_584;
	unk_588 = player->unk_588;
	m_unk134 = player->m_unk134;
	m_unk5dc = player->m_unk5dc;
	m_isRotating = player->m_isRotating;
	m_unk5e1 = player->m_unk5e1;
	m_hasGlow = player->m_hasGlow;
	m_isHidden = player->m_isHidden;
	m_hasGhostTrail = player->m_hasGhostTrail;
	m_unk62c = player->m_unk62c;
	m_unk630 = player->m_unk630;
	m_unk634 = player->m_unk634;
	m_unk638 = player->m_unk638;
	m_unk63c = player->m_unk63c;
	m_unk640 = player->m_unk640;
	m_unk644 = player->m_unk644;
	m_unk648 = player->m_unk648;
	m_unk658 = player->m_unk658;
	m_unk659 = player->m_unk659;
	m_unk65a = player->m_unk65a;
	m_unk65b = player->m_unk65b;
	m_playEffects = player->m_playEffects;
	m_unk65d = player->m_unk65d;
	m_unk65e = player->m_unk65e;
	m_unk65f = player->m_unk65f;
	m_unk688 = player->m_unk688;
	m_unk68c = player->m_unk68c;
	m_gv0096 = player->m_gv0096;
	m_gv0100 = player->m_gv0100;
	m_unk70c = player->m_unk70c;
	m_unk710 = player->m_unk710;
	m_playerStreak = player->m_playerStreak;
	m_hasCustomGlowColor = player->m_hasCustomGlowColor;
	m_isShip = player->m_isShip;
	m_isBird = player->m_isBird;
	m_isBall = player->m_isBall;
	m_isDart = player->m_isDart;
	m_isRobot = player->m_isRobot;
	m_isSpider = player->m_isSpider;
	m_isUpsideDown = player->m_isUpsideDown;
	m_isDead = player->m_isDead;
	m_isOnGround = player->m_isOnGround;
	m_isGoingLeft = player->m_isGoingLeft;
	m_isSideways = player->m_isSideways;
	m_isSwing = player->m_isSwing;
	m_unk7c8 = player->m_unk7c8;
	m_unk7cc = player->m_unk7cc;
	m_unk7d0 = player->m_unk7d0;
	m_isDashing = player->m_isDashing;
	m_vehicleSize = player->m_vehicleSize;
	m_playerSpeed = player->m_playerSpeed;
	m_unk7e0 = player->m_unk7e0;
	m_unk7e4 = player->m_unk7e4;
	m_unk7e8 = player->m_unk7e8;
	m_unk7f0 = player->m_unk7f0;
	m_unk7f8 = player->m_unk7f8;
	m_isLocked = player->m_isLocked;
	m_unka2b = player->m_unka2b;
	m_lastGroundedPos = player->m_lastGroundedPos;
	m_unk814 = player->m_unk814;
	m_unk815 = player->m_unk815;
	m_gamevar0060 = player->m_gamevar0060;
	m_gamevar0061 = player->m_gamevar0061;
	m_gamevar0062 = player->m_gamevar0062;
	m_unk838 = player->m_unk838;
	m_unk8ec = player->m_unk8ec;
	m_unk8f0 = player->m_unk8f0;
	m_unk8f4 = player->m_unk8f4;
	m_unk8f8 = player->m_unk8f8;
	m_unk904 = player->m_unk904;
	m_unk918 = player->m_unk918;
	m_unk91c = player->m_unk91c;
	m_unk948 = player->m_unk948;
	m_iconRequestID = player->m_iconRequestID;
	m_unk974 = player->m_unk974;
	m_unk978 = player->m_unk978;
	m_unk979 = player->m_unk979;
	m_unk97a = player->m_unk97a;
	m_unk97b = player->m_unk97b;

	m_objectSnappedTo = player->m_objectSnappedTo;

#ifndef GEODE_IS_ANDROID
	m_unk6a4 = player->m_unk6a4;
	m_unk828 = player->m_unk828;
#else
	std::copy(std::begin(player->m_unk6a4), std::end(player->m_unk6a4), std::begin(m_unk6a4));
	std::copy(std::begin(player->m_unk828), std::end(player->m_unk828), std::begin(m_unk828));
#endif
	m_unk880 = player->m_unk880;
	m_unk910 = player->m_unk910;
	m_unk924 = player->m_unk924;

	m_xVelocity = player->m_platformerXVelocity;
	m_yVelocity = player->m_yVelocity;
	m_xPosition = player->m_position.x;
	m_yPosition = player->m_position.y;
	m_rotationSpeed = player->m_rotationSpeed;
	m_rotation = player->getRotation();
	m_unk3c0 = player->m_unk3c0;
	m_unk3c8 = player->m_unk3c8;
	m_unk3d0 = player->m_unk3d0;
	m_unk3e0 = player->m_unk3e0;
	m_unk3e1 = player->m_unk3e1;
	m_unk3e2 = player->m_unk3e2;
	m_unk3e3 = player->m_unk3e3;
	m_platformerVelocityRelated = player->m_platformerVelocityRelated;
	m_slopeVelocityRelated = player->m_slopeVelocityRelated;

	m_gravityMod = player->m_gravityMod;
	m_isOnSlope = player->m_isOnSlope;
	m_wasOnSlope = player->m_wasOnSlope;
	m_affectedByForces = player->m_affectedByForces;
	m_holdingRight = player->m_holdingRight;
	m_holdingLeft = player->m_holdingLeft;
	m_isPlatformer = player->m_isPlatformer;
	m_slopeRelated = player->m_slopeRelated;
}

void PlayerSaveObject::apply(PlayerObject* player) {
	player->m_unk518 = m_unk518;
	player->m_unk51c = m_unk51c;
	player->m_unk51d = m_unk51d;
	player->m_unk51e = m_unk51e;
	player->m_unk51f = m_unk51f;
	player->m_unk523 = m_unk523;
	player->m_unk568 = m_unk568;
	player->unk_584 = unk_584;
	player->unk_588 = unk_588;
	player->m_unk134 = m_unk134;
	player->m_unk5dc = m_unk5dc;
	player->m_isRotating = m_isRotating;
	player->m_unk5e1 = m_unk5e1;
	player->m_hasGlow = m_hasGlow;
	player->m_isHidden = m_isHidden;
	player->m_hasGhostTrail = m_hasGhostTrail;
	player->m_unk62c = m_unk62c;
	player->m_unk630 = m_unk630;
	player->m_unk634 = m_unk634;
	player->m_unk638 = m_unk638;
	player->m_unk63c = m_unk63c;
	player->m_unk640 = m_unk640;
	player->m_unk644 = m_unk644;
	player->m_unk648 = m_unk648;
	player->m_unk658 = m_unk658;
	player->m_unk659 = m_unk659;
	player->m_unk65a = m_unk65a;
	player->m_unk65b = m_unk65b;
	player->m_playEffects = m_playEffects;
	player->m_unk65d = m_unk65d;
	player->m_unk65e = m_unk65e;
	player->m_unk65f = m_unk65f;
	player->m_unk688 = m_unk688;
	player->m_unk68c = m_unk68c;
	player->m_gv0096 = m_gv0096;
	player->m_gv0100 = m_gv0100;
	player->m_unk70c = m_unk70c;
	player->m_unk710 = m_unk710;
	player->m_playerStreak = m_playerStreak;
	player->m_hasCustomGlowColor = m_hasCustomGlowColor;
	player->m_isShip = m_isShip;
	player->m_isBird = m_isBird;
	player->m_isBall = m_isBall;
	player->m_isDart = m_isDart;
	player->m_isRobot = m_isRobot;
	player->m_isSpider = m_isSpider;
	player->m_isUpsideDown = m_isUpsideDown;
	player->m_isDead = m_isDead;
	player->m_isOnGround = m_isOnGround;
	player->m_isGoingLeft = m_isGoingLeft;
	player->m_isSideways = m_isSideways;
	player->m_isSwing = m_isSwing;
	player->m_unk7c8 = m_unk7c8;
	player->m_unk7cc = m_unk7cc;
	player->m_unk7d0 = m_unk7d0;
	player->m_isDashing = m_isDashing;
	player->m_vehicleSize = m_vehicleSize;
	player->m_playerSpeed = m_playerSpeed;
	player->m_unk7e0 = m_unk7e0;
	player->m_unk7e4 = m_unk7e4;
	player->m_unk7e8 = m_unk7e8;
	player->m_unk7f0 = m_unk7f0;
	player->m_unk7f8 = m_unk7f8;
	player->m_isLocked = m_isLocked;
	player->m_unka2b = m_unka2b;
	player->m_lastGroundedPos = m_lastGroundedPos;
	player->m_unk814 = m_unk814;
	player->m_unk815 = m_unk815;
	player->m_gamevar0060 = m_gamevar0060;
	player->m_gamevar0061 = m_gamevar0061;
	player->m_gamevar0062 = m_gamevar0062;
	player->m_unk838 = m_unk838;
	player->m_unk8ec = m_unk8ec;
	player->m_unk8f0 = m_unk8f0;
	player->m_unk8f4 = m_unk8f4;
	player->m_unk8f8 = m_unk8f8;
	player->m_unk904 = m_unk904;
	player->m_unk918 = m_unk918;
	player->m_unk91c = m_unk91c;
	player->m_unk948 = m_unk948;
	player->m_iconRequestID = m_iconRequestID;
	player->m_unk974 = m_unk974;
	player->m_unk978 = m_unk978;
	player->m_unk979 = m_unk979;
	player->m_unk97a = m_unk97a;
	player->m_unk97b = m_unk97b;

	player->m_objectSnappedTo = m_objectSnappedTo;

#ifndef GEODE_IS_ANDROID
	player->m_unk6a4 = m_unk6a4;
	player->m_unk828 = m_unk828;
#else
	std::copy(std::begin(m_unk6a4), std::end(m_unk6a4), std::begin(player->m_unk6a4));
	std::copy(std::begin(m_unk828), std::end(m_unk828), std::begin(player->m_unk828));
#endif
	player->m_unk880 = m_unk880;
	player->m_unk910 = m_unk910;
	player->m_unk924 = m_unk924;

	player->m_platformerXVelocity = m_xVelocity;
	player->m_yVelocity = m_yVelocity;
	player->setPositionX(m_xPosition);
	player->setPositionY(m_yPosition);
	player->m_rotationSpeed = m_rotationSpeed;
	player->setRotation(m_rotation);
	player->m_unk3c0 = m_unk3c0;
	player->m_unk3c8 = m_unk3c8;
	player->m_unk3d0 = m_unk3d0;
	player->m_unk3e0 = m_unk3e0;
	player->m_unk3e1 = m_unk3e1;
	player->m_unk3e2 = m_unk3e2;
	player->m_unk3e3 = m_unk3e3;
	player->m_platformerVelocityRelated = m_platformerVelocityRelated;
	player->m_slopeVelocityRelated = m_slopeVelocityRelated;

	player->m_gravityMod = m_gravityMod;
	player->m_isOnSlope = m_isOnSlope;
	player->m_wasOnSlope = m_wasOnSlope;
	player->m_affectedByForces = m_affectedByForces;
	player->m_holdingRight = m_holdingRight;
	player->m_holdingLeft = m_holdingLeft;
	player->m_isPlatformer = m_isPlatformer;
	player->m_slopeRelated = m_slopeRelated;
}