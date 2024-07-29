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
	m_wasTeleported = player->m_wasTeleported;
	m_fixGravityBug = player->m_fixGravityBug;
	m_reverseSync = player->m_reverseSync;
	m_yVelocityBeforeSlope = player->m_yVelocityBeforeSlope;
	m_dashX = player->m_dashX;
	m_dashY = player->m_dashY;
	m_dashAngle = player->m_dashAngle;
	m_dashStartTime = player->m_dashStartTime;
	m_dashRing = player->m_dashRing;
	m_slopeStartTime = player->m_slopeStartTime;
	m_justPlacedStreak = player->m_justPlacedStreak;
	m_maybeLastGroundObject = player->m_maybeLastGroundObject;
	m_collisionLogTop = player->m_collisionLogTop;
	m_collisionLogBottom = player->m_collisionLogBottom;
	m_collisionLogLeft = player->m_collisionLogLeft;
	m_collisionLogRight = player->m_collisionLogRight;
	m_lastCollisionBottom = player->m_lastCollisionBottom;
	m_lastCollisionTop = player->m_lastCollisionTop;
	m_lastCollisionLeft = player->m_lastCollisionLeft;
	m_lastCollisionRight = player->m_lastCollisionRight;
	m_unk50C = player->m_unk50C;
	m_unk510 = player->m_unk510;
	m_currentSlope2 = player->m_currentSlope2;
	m_preLastGroundObject = player->m_preLastGroundObject;
	m_slopeAngle = player->m_slopeAngle;
	m_slopeSlidingMaybeRotated = player->m_slopeSlidingMaybeRotated;
	m_quickCheckpointMode = player->m_quickCheckpointMode;
	m_collidedObject = player->m_collidedObject;
	m_lastGroundObject = player->m_lastGroundObject;
	m_collidingWithLeft = player->m_collidingWithLeft;
	m_collidingWithRight = player->m_collidingWithRight;
	m_maybeSavedPlayerFrame = player->m_maybeSavedPlayerFrame;
	m_scaleXRelated2 = player->m_scaleXRelated2;
	m_groundYVelocity = player->m_groundYVelocity;
	m_yVelocityRelated = player->m_yVelocityRelated;
	m_scaleXRelated3 = player->m_scaleXRelated3;
	m_scaleXRelated4 = player->m_scaleXRelated4;
	m_scaleXRelated5 = player->m_scaleXRelated5;
	m_isCollidingWithSlope = player->m_isCollidingWithSlope;
	m_isBallRotating = player->m_isBallRotating;
	m_unk669 = player->m_unk669;
	m_currentSlope3 = player->m_currentSlope3;
	m_currentSlope = player->m_currentSlope;
	unk_584 = player->unk_584;
	m_collidingWithSlopeId = player->m_collidingWithSlopeId;
	m_slopeFlipGravityRelated = player->m_slopeFlipGravityRelated;
	m_slopeAngleRadians = player->m_slopeAngleRadians;
	m_rotationSpeed = player->m_rotationSpeed;
	m_rotateSpeed = player->m_rotateSpeed;
	m_isRotating = player->m_isRotating;
	m_isBallRotating2 = player->m_isBallRotating2;
	m_isHidden = player->m_isHidden;
	m_ghostType = player->m_ghostType;
	m_speedMultiplier = player->m_speedMultiplier;
	m_yStart = player->m_yStart;
	m_gravity = player->m_gravity;
	m_trailingParticleLife = player->m_trailingParticleLife;
	m_unk648 = player->m_unk648;
	m_gameModeChangedTime = player->m_gameModeChangedTime;
	m_padRingRelated = player->m_padRingRelated;
	m_maybeReducedEffects = player->m_maybeReducedEffects;
	m_maybeIsFalling = player->m_maybeIsFalling;
	m_shouldTryPlacingCheckpoint = player->m_shouldTryPlacingCheckpoint;
	m_playEffects = player->m_playEffects;
	m_maybeCanRunIntoBlocks = player->m_maybeCanRunIntoBlocks;
	m_hasGroundParticles = player->m_hasGroundParticles;
	m_hasShipParticles = player->m_hasShipParticles;
	m_isOnGround3 = player->m_isOnGround3;
	m_checkpointTimeout = player->m_checkpointTimeout;
	m_lastCheckpointTime = player->m_lastCheckpointTime;
	m_lastJumpTime = player->m_lastJumpTime;
	m_lastFlipTime = player->m_lastFlipTime;
	m_flashTime = player->m_flashTime;
	m_flashRelated = player->m_flashRelated;
	m_flashRelated1 = player->m_flashRelated1;
	m_lastSpiderFlipTime = player->m_lastSpiderFlipTime;
	m_unkBool5 = player->m_unkBool5;
	m_maybeIsVehicleGlowing = player->m_maybeIsVehicleGlowing;
	m_gv0096 = player->m_gv0096;
	m_gv0100 = player->m_gv0100;
	m_accelerationOrSpeed = player->m_accelerationOrSpeed;
	m_snapDistance = player->m_snapDistance;
	m_ringJumpRelated = player->m_ringJumpRelated;
	m_objectSnappedTo = player->m_objectSnappedTo;
	m_onFlyCheckpointTries = player->m_onFlyCheckpointTries;
	m_playerStreak = player->m_playerStreak;
	m_slopeRotation = player->m_slopeRotation;
	m_currentSlopeYVelocity = player->m_currentSlopeYVelocity;
	m_unk3d0 = player->m_unk3d0;
	m_blackOrbRelated = player->m_blackOrbRelated;
	m_unk3e0 = player->m_unk3e0;
	m_unk3e1 = player->m_unk3e1;
	m_isAccelerating = player->m_isAccelerating;
	m_isCurrentSlopeTop = player->m_isCurrentSlopeTop;
	m_collidedTopMinY = player->m_collidedTopMinY;
	m_collidedBottomMaxY = player->m_collidedBottomMaxY;
	m_collidedLeftMaxX = player->m_collidedLeftMaxX;
	m_collidedRightMinX = player->m_collidedRightMinX;
	m_canPlaceCheckpoint = player->m_canPlaceCheckpoint;
	m_maybeIsColliding = player->m_maybeIsColliding;
	m_jumpBuffered = player->m_jumpBuffered;
	m_stateRingJump = player->m_stateRingJump;
	m_wasJumpBuffered = player->m_wasJumpBuffered;
	m_wasRobotJump = player->m_wasRobotJump;
	m_stateJumpBuffered = player->m_stateJumpBuffered;
	m_stateRingJump2 = player->m_stateRingJump2;
	m_touchedRing = player->m_touchedRing;
	m_touchedCustomRing = player->m_touchedCustomRing;
	m_touchedGravityPortal = player->m_touchedGravityPortal;
	m_maybeTouchedBreakableBlock = player->m_maybeTouchedBreakableBlock;
	m_jumpRelatedAC2 = player->m_jumpRelatedAC2;
	m_touchedPad = player->m_touchedPad;
	m_yVelocity = player->m_yVelocity;
	m_fallSpeed = player->m_fallSpeed;
	m_isOnSlope = player->m_isOnSlope;
	m_wasOnSlope = player->m_wasOnSlope;
	m_slopeVelocity = player->m_slopeVelocity;
	m_maybeUpsideDownSlope = player->m_maybeUpsideDownSlope;
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
	m_reverseRelated = player->m_reverseRelated;
	m_maybeReverseSpeed = player->m_maybeReverseSpeed;
	m_maybeReverseAcceleration = player->m_maybeReverseAcceleration;
	m_xVelocityRelated2 = player->m_xVelocityRelated2;
	m_isDashing = player->m_isDashing;
	m_unk9e8 = player->m_unk9e8;
	m_groundObjectMaterial = player->m_groundObjectMaterial;
	m_vehicleSize = player->m_vehicleSize;
	m_playerSpeed = player->m_playerSpeed;
	m_shipRotation = player->m_shipRotation;
	m_lastPortalPos = player->m_lastPortalPos;
	m_unkUnused3 = player->m_unkUnused3;
	m_isOnGround2 = player->m_isOnGround2;
	m_lastLandTime = player->m_lastLandTime;
	m_platformerVelocityRelated = player->m_platformerVelocityRelated;
	m_maybeIsBoosted = player->m_maybeIsBoosted;
	m_scaleXRelatedTime = player->m_scaleXRelatedTime;
	m_decreaseBoostSlide = player->m_decreaseBoostSlide;
	m_unkA29 = player->m_unkA29;
	m_isLocked = player->m_isLocked;
	m_controlsDisabled = player->m_controlsDisabled;
	m_lastGroundedPos = player->m_lastGroundedPos;
	m_touchingRings = player->m_touchingRings;
	m_lastActivatedPortal = player->m_lastActivatedPortal;
	m_hasEverJumped = player->m_hasEverJumped;
	m_ringOrStreakRelated = player->m_ringOrStreakRelated;
	m_isSecondPlayer = player->m_isSecondPlayer;
	m_unkA99 = player->m_unkA99;
	m_totalTime = player->m_totalTime;
	m_isBeingSpawnedByDualPortal = player->m_isBeingSpawnedByDualPortal;
	m_unkAAC = player->m_unkAAC;
	m_unkAngle1 = player->m_unkAngle1;
	m_yVelocityRelated3 = player->m_yVelocityRelated3;
	m_gamevar0060 = player->m_gamevar0060;
	m_swapColors = player->m_swapColors;
	m_gamevar0062 = player->m_gamevar0062;
	m_followRelated = player->m_followRelated;
	m_unk838 = player->m_unk838;
	m_stateOnGround = player->m_stateOnGround;
	m_stateUnk = player->m_stateUnk;
	m_stateNoStickX = player->m_stateNoStickX;
	m_stateNoStickY = player->m_stateNoStickY;
	m_stateUnk2 = player->m_stateUnk2;
	m_stateBoostX = player->m_stateBoostX;
	m_stateBoostY = player->m_stateBoostY;
	m_maybeStateForce2 = player->m_maybeStateForce2;
	m_stateScale = player->m_stateScale;
	m_platformerXVelocity = player->m_platformerXVelocity;
	m_holdingRight = player->m_holdingRight;
	m_holdingLeft = player->m_holdingLeft;
	m_leftPressedFirst = player->m_leftPressedFirst;
	m_scaleXRelated = player->m_scaleXRelated;
	m_maybeHasStopped = player->m_maybeHasStopped;
	m_xVelocityRelated = player->m_xVelocityRelated;
	m_maybeGoingCorrectSlopeDirection = player->m_maybeGoingCorrectSlopeDirection;
	m_isSliding = player->m_isSliding;
	m_maybeSlopeForce = player->m_maybeSlopeForce;
	m_isOnIce = player->m_isOnIce;
	m_physDeltaRelated = player->m_physDeltaRelated;
	m_isOnGround4 = player->m_isOnGround4;
	m_maybeSlidingTime = player->m_maybeSlidingTime;
	m_maybeSlidingStartTime = player->m_maybeSlidingStartTime;
	m_changedDirectionsTime = player->m_changedDirectionsTime;
	m_slopeEndTime = player->m_slopeEndTime;
	m_isMoving = player->m_isMoving;
	m_platformerMovingLeft = player->m_platformerMovingLeft;
	m_platformerMovingRight = player->m_platformerMovingRight;
	m_isSlidingRight = player->m_isSlidingRight;
	m_maybeChangedDirectionAngle = player->m_maybeChangedDirectionAngle;
	m_unkUnused2 = player->m_unkUnused2;
	m_isPlatformer = player->m_isPlatformer;
	m_stateNoAutoJump = player->m_stateNoAutoJump;
	m_stateDartSlide = player->m_stateDartSlide;
	m_stateHitHead = player->m_stateHitHead;
	m_stateFlipGravity = player->m_stateFlipGravity;
	m_gravityMod = player->m_gravityMod;
	m_stateForce = player->m_stateForce;
	m_stateForceVector = player->m_stateForceVector;
	m_affectedByForces = player->m_affectedByForces;
	m_somethingPlayerSpeedTime = player->m_somethingPlayerSpeedTime;
	m_playerSpeedAC = player->m_playerSpeedAC;
	m_fixRobotJump = player->m_fixRobotJump;
	m_inputsLocked = player->m_inputsLocked;
	m_currentRobotAnimation = player->m_currentRobotAnimation;
	m_gv0123 = player->m_gv0123;
	m_iconRequestID = player->m_iconRequestID;
	m_unk958 = player->m_unk958;
	m_unkUnused = player->m_unkUnused;
	m_isOutOfBounds = player->m_isOutOfBounds;
	m_fallStartY = player->m_fallStartY;
	m_disablePlayerSqueeze = player->m_disablePlayerSqueeze;
	m_robotHasRun3 = player->m_robotHasRun3;
	m_robotHasRun2 = player->m_robotHasRun2;
	m_item20 = player->m_item20;
	m_ignoreDamage = player->m_ignoreDamage;
	m_enable22Changes = player->m_enable22Changes;

	//These fields can cause issues on android so I seperate them
#ifndef GEODE_IS_ANDROID
	m_rotateObjectsRelated = player->m_rotateObjectsRelated;
	m_maybeRotatedObjectsMap = player->m_maybeRotatedObjectsMap;
	m_ringRelatedSet = player->m_ringRelatedSet;
	m_touchedRings = player->m_touchedRings;
#else
	std::copy(std::begin(player->m_rotateObjectsRelated), std::end(player->m_rotateObjectsRelated), std::begin(m_rotateObjectsRelated));
	std::copy(std::begin(player->m_maybeRotatedObjectsMap), std::end(player->m_maybeRotatedObjectsMap), std::begin(m_maybeRotatedObjectsMap));
	std::copy(std::begin(player->m_ringRelatedSet), std::end(player->m_ringRelatedSet), std::begin(m_ringRelatedSet));
	std::copy(std::begin(player->m_touchedRings), std::end(player->m_touchedRings), std::begin(m_touchedRings));
#endif
	m_playerFollowFloats = player->m_playerFollowFloats;
	m_jumpPadRelated = player->m_jumpPadRelated;
	m_holdingButtons = player->m_holdingButtons;

	//Custom fields
	m_xPosition = player->getPositionX();
	m_yPosition = player->getPositionY();
}

void PlayerSaveObject::apply(PlayerObject* player) {
	player->m_wasTeleported = m_wasTeleported;
	player->m_fixGravityBug = m_fixGravityBug;
	player->m_reverseSync = m_reverseSync;
	player->m_yVelocityBeforeSlope = m_yVelocityBeforeSlope;
	player->m_dashX = m_dashX;
	player->m_dashY = m_dashY;
	player->m_dashAngle = m_dashAngle;
	player->m_dashStartTime = m_dashStartTime;
	player->m_dashRing = m_dashRing;
	player->m_slopeStartTime = m_slopeStartTime;
	player->m_justPlacedStreak = m_justPlacedStreak;
	player->m_maybeLastGroundObject = m_maybeLastGroundObject;
	player->m_collisionLogTop = m_collisionLogTop;
	player->m_collisionLogBottom = m_collisionLogBottom;
	player->m_collisionLogLeft = m_collisionLogLeft;
	player->m_collisionLogRight = m_collisionLogRight;
	player->m_lastCollisionBottom = m_lastCollisionBottom;
	player->m_lastCollisionTop = m_lastCollisionTop;
	player->m_lastCollisionLeft = m_lastCollisionLeft;
	player->m_lastCollisionRight = m_lastCollisionRight;
	player->m_unk50C = m_unk50C;
	player->m_unk510 = m_unk510;
	player->m_currentSlope2 = m_currentSlope2;
	player->m_preLastGroundObject = m_preLastGroundObject;
	player->m_slopeAngle = m_slopeAngle;
	player->m_slopeSlidingMaybeRotated = m_slopeSlidingMaybeRotated;
	player->m_quickCheckpointMode = m_quickCheckpointMode;
	player->m_collidedObject = m_collidedObject;
	player->m_lastGroundObject = m_lastGroundObject;
	player->m_collidingWithLeft = m_collidingWithLeft;
	player->m_collidingWithRight = m_collidingWithRight;
	player->m_maybeSavedPlayerFrame = m_maybeSavedPlayerFrame;
	player->m_scaleXRelated2 = m_scaleXRelated2;
	player->m_groundYVelocity = m_groundYVelocity;
	player->m_yVelocityRelated = m_yVelocityRelated;
	player->m_scaleXRelated3 = m_scaleXRelated3;
	player->m_scaleXRelated4 = m_scaleXRelated4;
	player->m_scaleXRelated5 = m_scaleXRelated5;
	player->m_isCollidingWithSlope = m_isCollidingWithSlope;
	player->m_isBallRotating = m_isBallRotating;
	player->m_unk669 = m_unk669;
	player->m_currentSlope3 = m_currentSlope3;
	player->m_currentSlope = m_currentSlope;
	player->unk_584 = unk_584;
	player->m_collidingWithSlopeId = m_collidingWithSlopeId;
	player->m_slopeFlipGravityRelated = m_slopeFlipGravityRelated;
	player->m_slopeAngleRadians = m_slopeAngleRadians;
	player->m_rotationSpeed = m_rotationSpeed;
	player->m_rotateSpeed = m_rotateSpeed;
	player->m_isRotating = m_isRotating;
	player->m_isBallRotating2 = m_isBallRotating2;
	player->m_isHidden = m_isHidden;
	player->m_ghostType = m_ghostType;
	player->m_speedMultiplier = m_speedMultiplier;
	player->m_yStart = m_yStart;
	player->m_gravity = m_gravity;
	player->m_trailingParticleLife = m_trailingParticleLife;
	player->m_unk648 = m_unk648;
	player->m_gameModeChangedTime = m_gameModeChangedTime;
	player->m_padRingRelated = m_padRingRelated;
	player->m_maybeReducedEffects = m_maybeReducedEffects;
	player->m_maybeIsFalling = m_maybeIsFalling;
	player->m_shouldTryPlacingCheckpoint = m_shouldTryPlacingCheckpoint;
	player->m_playEffects = m_playEffects;
	player->m_maybeCanRunIntoBlocks = m_maybeCanRunIntoBlocks;
	player->m_hasGroundParticles = m_hasGroundParticles;
	player->m_hasShipParticles = m_hasShipParticles;
	player->m_isOnGround3 = m_isOnGround3;
	player->m_checkpointTimeout = m_checkpointTimeout;
	player->m_lastCheckpointTime = m_lastCheckpointTime;
	player->m_lastJumpTime = m_lastJumpTime;
	player->m_lastFlipTime = m_lastFlipTime;
	player->m_flashTime = m_flashTime;
	player->m_flashRelated = m_flashRelated;
	player->m_flashRelated1 = m_flashRelated1;
	player->m_lastSpiderFlipTime = m_lastSpiderFlipTime;
	player->m_unkBool5 = m_unkBool5;
	player->m_maybeIsVehicleGlowing = m_maybeIsVehicleGlowing;
	player->m_gv0096 = m_gv0096;
	player->m_gv0100 = m_gv0100;
	player->m_accelerationOrSpeed = m_accelerationOrSpeed;
	player->m_snapDistance = m_snapDistance;
	player->m_ringJumpRelated = m_ringJumpRelated;
	player->m_objectSnappedTo = m_objectSnappedTo;
	player->m_onFlyCheckpointTries = m_onFlyCheckpointTries;
	player->m_playerStreak = m_playerStreak;
	player->m_slopeRotation = m_slopeRotation;
	player->m_currentSlopeYVelocity = m_currentSlopeYVelocity;
	player->m_unk3d0 = m_unk3d0;
	player->m_blackOrbRelated = m_blackOrbRelated;
	player->m_unk3e0 = m_unk3e0;
	player->m_unk3e1 = m_unk3e1;
	player->m_isAccelerating = m_isAccelerating;
	player->m_isCurrentSlopeTop = m_isCurrentSlopeTop;
	player->m_collidedTopMinY = m_collidedTopMinY;
	player->m_collidedBottomMaxY = m_collidedBottomMaxY;
	player->m_collidedLeftMaxX = m_collidedLeftMaxX;
	player->m_collidedRightMinX = m_collidedRightMinX;
	player->m_canPlaceCheckpoint = m_canPlaceCheckpoint;
	player->m_maybeIsColliding = m_maybeIsColliding;
	player->m_jumpBuffered = m_jumpBuffered;
	player->m_stateRingJump = m_stateRingJump;
	player->m_wasJumpBuffered = m_wasJumpBuffered;
	player->m_wasRobotJump = m_wasRobotJump;
	player->m_stateJumpBuffered = m_stateJumpBuffered;
	player->m_stateRingJump2 = m_stateRingJump2;
	player->m_touchedRing = m_touchedRing;
	player->m_touchedCustomRing = m_touchedCustomRing;
	player->m_touchedGravityPortal = m_touchedGravityPortal;
	player->m_maybeTouchedBreakableBlock = m_maybeTouchedBreakableBlock;
	player->m_jumpRelatedAC2 = m_jumpRelatedAC2;
	player->m_touchedPad = m_touchedPad;
	player->m_yVelocity = m_yVelocity;
	player->m_fallSpeed = m_fallSpeed;
	player->m_isOnSlope = m_isOnSlope;
	player->m_wasOnSlope = m_wasOnSlope;
	player->m_slopeVelocity = m_slopeVelocity;
	player->m_maybeUpsideDownSlope = m_maybeUpsideDownSlope;
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
	player->m_reverseRelated = m_reverseRelated;
	player->m_maybeReverseSpeed = m_maybeReverseSpeed;
	player->m_maybeReverseAcceleration = m_maybeReverseAcceleration;
	player->m_xVelocityRelated2 = m_xVelocityRelated2;
	player->m_isDashing = m_isDashing;
	player->m_unk9e8 = m_unk9e8;
	player->m_groundObjectMaterial = m_groundObjectMaterial;
	player->m_vehicleSize = m_vehicleSize;
	player->m_playerSpeed = m_playerSpeed;
	player->m_shipRotation = m_shipRotation;
	player->m_lastPortalPos = m_lastPortalPos;
	player->m_unkUnused3 = m_unkUnused3;
	player->m_isOnGround2 = m_isOnGround2;
	player->m_lastLandTime = m_lastLandTime;
	player->m_platformerVelocityRelated = m_platformerVelocityRelated;
	player->m_maybeIsBoosted = m_maybeIsBoosted;
	player->m_scaleXRelatedTime = m_scaleXRelatedTime;
	player->m_decreaseBoostSlide = m_decreaseBoostSlide;
	player->m_unkA29 = m_unkA29;
	player->m_isLocked = m_isLocked;
	player->m_controlsDisabled = m_controlsDisabled;
	player->m_lastGroundedPos = m_lastGroundedPos;
	player->m_touchingRings = m_touchingRings;
	player->m_lastActivatedPortal = m_lastActivatedPortal;
	player->m_hasEverJumped = m_hasEverJumped;
	player->m_ringOrStreakRelated = m_ringOrStreakRelated;
	player->m_isSecondPlayer = m_isSecondPlayer;
	player->m_unkA99 = m_unkA99;
	player->m_totalTime = m_totalTime;
	player->m_isBeingSpawnedByDualPortal = m_isBeingSpawnedByDualPortal;
	player->m_unkAAC = m_unkAAC;
	player->m_unkAngle1 = m_unkAngle1;
	player->m_yVelocityRelated3 = m_yVelocityRelated3;
	player->m_gamevar0060 = m_gamevar0060;
	player->m_swapColors = m_swapColors;
	player->m_gamevar0062 = m_gamevar0062;
	player->m_followRelated = m_followRelated;
	player->m_unk838 = m_unk838;
	player->m_stateOnGround = m_stateOnGround;
	player->m_stateUnk = m_stateUnk;
	player->m_stateNoStickX = m_stateNoStickX;
	player->m_stateNoStickY = m_stateNoStickY;
	player->m_stateUnk2 = m_stateUnk2;
	player->m_stateBoostX = m_stateBoostX;
	player->m_stateBoostY = m_stateBoostY;
	player->m_maybeStateForce2 = m_maybeStateForce2;
	player->m_stateScale = m_stateScale;
	player->m_platformerXVelocity = m_platformerXVelocity;
	player->m_holdingRight = m_holdingRight;
	player->m_holdingLeft = m_holdingLeft;
	player->m_leftPressedFirst = m_leftPressedFirst;
	player->m_scaleXRelated = m_scaleXRelated;
	player->m_maybeHasStopped = m_maybeHasStopped;
	player->m_xVelocityRelated = m_xVelocityRelated;
	player->m_maybeGoingCorrectSlopeDirection = m_maybeGoingCorrectSlopeDirection;
	player->m_isSliding = m_isSliding;
	player->m_maybeSlopeForce = m_maybeSlopeForce;
	player->m_isOnIce = m_isOnIce;
	player->m_physDeltaRelated = m_physDeltaRelated;
	player->m_isOnGround4 = m_isOnGround4;
	player->m_maybeSlidingTime = m_maybeSlidingTime;
	player->m_maybeSlidingStartTime = m_maybeSlidingStartTime;
	player->m_changedDirectionsTime = m_changedDirectionsTime;
	player->m_slopeEndTime = m_slopeEndTime;
	player->m_isMoving = m_isMoving;
	player->m_platformerMovingLeft = m_platformerMovingLeft;
	player->m_platformerMovingRight = m_platformerMovingRight;
	player->m_isSlidingRight = m_isSlidingRight;
	player->m_maybeChangedDirectionAngle = m_maybeChangedDirectionAngle;
	player->m_unkUnused2 = m_unkUnused2;
	player->m_isPlatformer = m_isPlatformer;
	player->m_stateNoAutoJump = m_stateNoAutoJump;
	player->m_stateDartSlide = m_stateDartSlide;
	player->m_stateHitHead = m_stateHitHead;
	player->m_stateFlipGravity = m_stateFlipGravity;
	player->m_gravityMod = m_gravityMod;
	player->m_stateForce = m_stateForce;
	player->m_stateForceVector = m_stateForceVector;
	player->m_affectedByForces = m_affectedByForces;
	player->m_somethingPlayerSpeedTime = m_somethingPlayerSpeedTime;
	player->m_playerSpeedAC = m_playerSpeedAC;
	player->m_fixRobotJump = m_fixRobotJump;
	player->m_inputsLocked = m_inputsLocked;
	player->m_currentRobotAnimation = m_currentRobotAnimation;
	player->m_gv0123 = m_gv0123;
	player->m_iconRequestID = m_iconRequestID;
	player->m_unk958 = m_unk958;
	player->m_unkUnused = m_unkUnused;
	player->m_isOutOfBounds = m_isOutOfBounds;
	player->m_fallStartY = m_fallStartY;
	player->m_disablePlayerSqueeze = m_disablePlayerSqueeze;
	player->m_robotHasRun3 = m_robotHasRun3;
	player->m_robotHasRun2 = m_robotHasRun2;
	player->m_item20 = m_item20;
	player->m_ignoreDamage = m_ignoreDamage;
	player->m_enable22Changes = m_enable22Changes;

	//These fields can cause issues on android so I seperate them
#ifndef GEODE_IS_ANDROID
	player->m_rotateObjectsRelated = m_rotateObjectsRelated;
	player->m_maybeRotatedObjectsMap = m_maybeRotatedObjectsMap;
	player->m_ringRelatedSet = m_ringRelatedSet;
	player->m_touchedRings = m_touchedRings;
#else
	std::copy(std::begin(m_rotateObjectsRelated), std::end(m_rotateObjectsRelated), std::begin(player->m_rotateObjectsRelated));
	std::copy(std::begin(m_maybeRotatedObjectsMap), std::end(m_maybeRotatedObjectsMap), std::begin(player->m_maybeRotatedObjectsMap));
	std::copy(std::begin(m_ringRelatedSet), std::end(m_ringRelatedSet), std::begin(player->m_ringRelatedSet));
	std::copy(std::begin(m_touchedRings), std::end(m_touchedRings), std::begin(player->m_touchedRings));
#endif
	player->m_playerFollowFloats = m_playerFollowFloats;
	player->m_jumpPadRelated = m_jumpPadRelated;
	player->m_holdingButtons = m_holdingButtons;

	//Custom fields
	player->setPositionX(m_xPosition);
	player->setPositionY(m_yPosition);
}