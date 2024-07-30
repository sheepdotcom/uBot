#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct playerData {
	float xPos;
	float yPos;
	double xVel;
	double yVel;
};

struct macroData {
	bool isPlayer1;
	int button;
	int frame;
	bool holding;
	playerData pData;
};

//This exists so the bot knows which fixes are enabled (for compatibility with converting macros) and extra data to store
struct infoData {
	bool posFix;
	bool yVelFix;
	bool xVelFix;
	bool platformer;
};

enum state {
	off,
	recording,
	playing
};

//Might make things easier
enum BotFileError {
	GenericError,
	Success,
	EmptyFileName,
	InvalidFileName,
	UnableToOpenFile,
	OlderVersion //This one is probably not gonna be used because I want backwards compatiblity
};

class uwuBot {
public:
	static inline uwuBot* catgirl = nullptr;

	CCLabelBMFont* frameLabel = nullptr;
	CCLabelBMFont* stateLabel = nullptr;

	state m_state = off;
	infoData m_infoData;
	std::vector<macroData> m_macroData;
	std::string m_formatVersion = "1"; //Version of the format, most likely older versions of the mod either wont accept newer formats or may give a popup warning about trying to parse a newer version, while newer versions may accept older format versions.
	std::string m_gdVersion = "2.206";
	int m_currentAction;

	int getCurrentFrame();
	void recordInput(bool isPlayer1, int button, int frame, bool holding, playerData playerData);
	void clearInputsAfterFrame(int frame);
	void updateInfo(bool posFix, bool yVelFix, bool xVelFix, bool platformer);
	BotFileError saveMacro(std::string name);
	BotFileError loadMacro(std::string name);
	void clearState();
	void updateLabels();
	void resetAudioSpeed();
};

class PlayerSaveObject {
public:
	PlayerSaveObject() = default;
	PlayerSaveObject(PlayerObject* player);

	void apply(PlayerObject* player);

private:
	//I copied these from the bindings then manually filtered out ones I don't need (Updated from commit 5d262f1)
	bool m_wasTeleported;
	bool m_fixGravityBug;
	bool m_reverseSync;
	double m_yVelocityBeforeSlope;
	double m_dashX;
	double m_dashY;
	double m_dashAngle;
	double m_dashStartTime;
	DashRingObject* m_dashRing;
	double m_slopeStartTime;
	bool m_justPlacedStreak;
	GameObject* m_maybeLastGroundObject;
	cocos2d::CCDictionary* m_collisionLogTop;
	cocos2d::CCDictionary* m_collisionLogBottom;
	cocos2d::CCDictionary* m_collisionLogLeft;
	cocos2d::CCDictionary* m_collisionLogRight;
	int m_lastCollisionBottom;
	int m_lastCollisionTop;
	int m_lastCollisionLeft;
	int m_lastCollisionRight;
	int m_unk50C;
	int m_unk510;
	GameObject* m_currentSlope2;
	GameObject* m_preLastGroundObject;
	float m_slopeAngle;
	bool m_slopeSlidingMaybeRotated;
	bool m_quickCheckpointMode;
	GameObject* m_collidedObject;
	GameObject* m_lastGroundObject;
	GameObject* m_collidingWithLeft;
	GameObject* m_collidingWithRight;
	int m_maybeSavedPlayerFrame;
	double m_scaleXRelated2;
	double m_groundYVelocity;
	double m_yVelocityRelated;
	double m_scaleXRelated3;
	double m_scaleXRelated4;
	double m_scaleXRelated5;
	bool m_isCollidingWithSlope;
	bool m_isBallRotating;
	bool m_unk669;
	GameObject* m_currentSlope3;
	GameObject* m_currentSlope;
	double unk_584;
	int m_collidingWithSlopeId;
	bool m_slopeFlipGravityRelated;
	float m_slopeAngleRadians;
	float m_rotationSpeed;
	float m_rotateSpeed;
	bool m_isRotating;
	bool m_isBallRotating2;
	bool m_isHidden;
	GhostType m_ghostType;
	double m_speedMultiplier;
	double m_yStart;
	double m_gravity;
	float m_trailingParticleLife;
	float m_unk648;
	double m_gameModeChangedTime;
	bool m_padRingRelated;
	bool m_maybeReducedEffects;
	bool m_maybeIsFalling;
	bool m_shouldTryPlacingCheckpoint;
	bool m_playEffects;
	bool m_maybeCanRunIntoBlocks;
	bool m_hasGroundParticles;
	bool m_hasShipParticles;
	bool m_isOnGround3;
	bool m_checkpointTimeout;
	double m_lastCheckpointTime;
	double m_lastJumpTime;
	double m_lastFlipTime;
	double m_flashTime;
	float m_flashRelated;
	float m_flashRelated1;
	double m_lastSpiderFlipTime;
	bool m_unkBool5;
	bool m_maybeIsVehicleGlowing;
	bool m_gv0096;
	bool m_gv0100;
	double m_accelerationOrSpeed;
	double m_snapDistance;
	bool m_ringJumpRelated;
	GameObject* m_objectSnappedTo;
	int m_onFlyCheckpointTries;
	int m_playerStreak;
	double m_slopeRotation;
	double m_currentSlopeYVelocity;
	double m_unk3d0;
	double m_blackOrbRelated;
	bool m_unk3e0;
	bool m_unk3e1;
	bool m_isAccelerating;
	bool m_isCurrentSlopeTop;
	double m_collidedTopMinY;
	double m_collidedBottomMaxY;
	double m_collidedLeftMaxX;
	double m_collidedRightMinX;
	bool m_canPlaceCheckpoint;
	bool m_maybeIsColliding;
	bool m_jumpBuffered;
	bool m_stateRingJump;
	bool m_wasJumpBuffered;
	bool m_wasRobotJump;
	unsigned char m_stateJumpBuffered;
	bool m_stateRingJump2;
	bool m_touchedRing;
	bool m_touchedCustomRing;
	bool m_touchedGravityPortal;
	bool m_maybeTouchedBreakableBlock;
	geode::SeedValueRSV m_jumpRelatedAC2;
	bool m_touchedPad;
	double m_yVelocity;
	double m_fallSpeed;
	bool m_isOnSlope;
	bool m_wasOnSlope;
	float m_slopeVelocity;
	bool m_maybeUpsideDownSlope;
	bool m_isShip;
	bool m_isBird;
	bool m_isBall;
	bool m_isDart;
	bool m_isRobot;
	bool m_isSpider;
	bool m_isUpsideDown;
	bool m_isDead;
	bool m_isOnGround;
	bool m_isGoingLeft;
	bool m_isSideways;
	bool m_isSwing;
	int m_reverseRelated;
	double m_maybeReverseSpeed;
	double m_maybeReverseAcceleration;
	float m_xVelocityRelated2;
	bool m_isDashing;
	int m_unk9e8;
	int m_groundObjectMaterial;
	float m_vehicleSize;
	float m_playerSpeed;
	cocos2d::CCPoint m_shipRotation;
	cocos2d::CCPoint m_lastPortalPos;
	float m_unkUnused3;
	bool m_isOnGround2;
	double m_lastLandTime;
	float m_platformerVelocityRelated;
	bool m_maybeIsBoosted;
	double m_scaleXRelatedTime;
	bool m_decreaseBoostSlide;
	bool m_unkA29;
	bool m_isLocked;
	bool m_controlsDisabled;
	cocos2d::CCPoint m_lastGroundedPos;
	cocos2d::CCArray* m_touchingRings;
	GameObject* m_lastActivatedPortal;
	bool m_hasEverJumped;
	bool m_ringOrStreakRelated;
	bool m_isSecondPlayer;
	bool m_unkA99;
	double m_totalTime;
	bool m_isBeingSpawnedByDualPortal;
	float m_unkAAC;
	float m_unkAngle1;
	float m_yVelocityRelated3;
	bool m_gamevar0060;
	bool m_swapColors;
	bool m_gamevar0062;
	int m_followRelated;
	float m_unk838;
	int m_stateOnGround;
	unsigned char m_stateUnk;
	unsigned char m_stateNoStickX;
	unsigned char m_stateNoStickY;
	unsigned char m_stateUnk2;
	int m_stateBoostX;
	int m_stateBoostY;
	int m_maybeStateForce2;
	int m_stateScale;
	double m_platformerXVelocity;
	//bool m_holdingRight;
	//bool m_holdingLeft;
	bool m_leftPressedFirst;
	double m_scaleXRelated;
	bool m_maybeHasStopped;
	float m_xVelocityRelated;
	bool m_maybeGoingCorrectSlopeDirection;
	bool m_isSliding;
	double m_maybeSlopeForce;
	bool m_isOnIce;
	double m_physDeltaRelated;
	bool m_isOnGround4;
	int m_maybeSlidingTime;
	double m_maybeSlidingStartTime;
	double m_changedDirectionsTime;
	double m_slopeEndTime;
	bool m_isMoving;
	bool m_platformerMovingLeft;
	bool m_platformerMovingRight;
	bool m_isSlidingRight;
	double m_maybeChangedDirectionAngle;
	double m_unkUnused2;
	bool m_isPlatformer;
	int m_stateNoAutoJump;
	int m_stateDartSlide;
	int m_stateHitHead;
	int m_stateFlipGravity;
	float m_gravityMod;
	int m_stateForce;
	cocos2d::CCPoint m_stateForceVector;
	bool m_affectedByForces;
	float m_somethingPlayerSpeedTime;
	float m_playerSpeedAC;
	bool m_fixRobotJump;
	bool m_inputsLocked;
	gd::string m_currentRobotAnimation;
	bool m_gv0123;
	int m_iconRequestID;
	cocos2d::CCArray* m_unk958;
	int m_unkUnused;
	bool m_isOutOfBounds;
	float m_fallStartY;
	bool m_disablePlayerSqueeze;
	bool m_robotHasRun3;
	bool m_robotHasRun2;
	bool m_item20;
	bool m_ignoreDamage;
	bool m_enable22Changes;

	//These fields can cause issues on android so I seperate them
	gd::unordered_map<int, GJPointDouble> m_rotateObjectsRelated;
	gd::unordered_map<int, GameObject*> m_maybeRotatedObjectsMap;
	gd::unordered_set<int> m_ringRelatedSet;
	gd::unordered_set<int> m_touchedRings;
	gd::vector<float> m_playerFollowFloats;
	gd::map<int, bool> m_jumpPadRelated;
	//gd::map<int, bool> m_holdingButtons; //Disabled until I make the fix/feature that ybot has (pre-input in the PauseLayer)

	//Custom fields
	float m_xPosition;
	float m_yPosition;
};

class CheckpointSave {
public:
	CheckpointSave() = default;

	CheckpointSave(PlayerObject* p1, PlayerObject* p2) {
		m_playerSave1 = PlayerSaveObject(p1);
		if (p2) m_playerSave2 = PlayerSaveObject(p2);
	}

	void apply(PlayerObject* p1, PlayerObject* p2) {
		m_playerSave1.apply(p1);
		if (p2) m_playerSave2.apply(p2);
	}

private:
	PlayerSaveObject m_playerSave1;
	PlayerSaveObject m_playerSave2;
};