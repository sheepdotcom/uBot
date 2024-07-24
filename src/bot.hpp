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
};

class PlayerSaveObject {
public:
	PlayerSaveObject() = default;
	PlayerSaveObject(PlayerObject* player);

	void apply(PlayerObject* player);

private:
	//I copied these from the bindings then manually filtered out ones I don't need
	float m_unk518;
	bool m_unk51c;
	bool m_unk51d;
	bool m_unk51e;
	bool m_unk51f;
	float m_unk523;
	float m_unk568;
	float unk_584;
	int unk_588; // seems to always be 0, but when you respawn it's -1 until you move at least 1 block from the respawn place
	bool m_unk134;
	float m_unk5dc;
	bool m_isRotating;
	bool m_unk5e1;
	bool m_hasGlow;
	bool m_isHidden;
	int m_hasGhostTrail;
	float m_unk62c;
	int m_unk630;
	float m_unk634;
	int m_unk638;
	float m_unk63c;
	int m_unk640;
	float m_unk644;
	float m_unk648;
	bool m_unk658;
	bool m_unk659;
	bool m_unk65a;
	bool m_unk65b; // midair ??
	bool m_playEffects; // controls whether to play some things like death effects, spider dash anim
	bool m_unk65d; // walking ???
	bool m_unk65e;
	bool m_unk65f;
	float m_unk688;
	float m_unk68c;
	bool m_gv0096; // = GameManager::getGameVariable("0096")
	bool m_gv0100; // = GameManager::getGameVariable("0100")
	int m_unk6c0;
	float m_unk70c;
	float m_unk710;
	int m_playerStreak;
	bool m_hasCustomGlowColor;
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
	float m_unk7c8;
	float m_unk7cc; // unsure if float
	float m_unk7d0;
	bool m_isDashing;
	float m_vehicleSize;
	float m_playerSpeed;
	float m_unk7e0;
	float m_unk7e4;
	cocos2d::CCPoint m_unk7e8;
	cocos2d::CCPoint m_unk7f0; // maybe m_lastPortalPos
	bool m_unk7f8;
	bool m_isLocked;
	bool m_unka2b;
	cocos2d::CCPoint m_lastGroundedPos;
	bool m_unk814;
	bool m_unk815;
	bool m_gamevar0060; // used in init
	bool m_gamevar0061;
	bool m_gamevar0062;
	float m_unk838;
	int m_unk8ec;
	int m_unk8f0;
	int m_unk8f4;
	int m_unk8f8;
	cocos2d::CCPoint m_unk904;
	float m_unk918; // increments whenever you're midiar?
	float m_unk91c;
	bool m_unk948; // = getGameVariable("0123")
	int m_iconRequestID;
	float m_unk974;
	bool m_unk978;
	bool m_unk979; // = isItemEnabled(0xc, 0x12);
	bool m_unk97a; // = isItemEnabled(0xc, 0x13);
	bool m_unk97b; // = isItemEnabled(0xc, 0x14);

	GameObject* m_objectSnappedTo = nullptr;

	gd::unordered_set<int> m_unk6a4;
	gd::unordered_set<int> m_unk828;
	gd::vector<float> m_unk880;
	gd::map<int, bool> m_unk910;
	gd::map<int, bool> m_unk924;

	double m_xVelocity;
	double m_yVelocity;
	float m_xPosition;
	float m_yPosition;
	float m_rotationSpeed;
	float m_rotation;
	double m_unk3c0;
	double m_unk3c8;
	double m_unk3d0;
	bool m_unk3e0;
	bool m_unk3e1;
	bool m_unk3e2; // used to be m_blackOrbRelated
	bool m_unk3e3;
	float m_platformerVelocityRelated;
	float m_slopeVelocityRelated;

	float m_gravityMod;
	bool m_isOnSlope;
	bool m_wasOnSlope;
	bool m_affectedByForces;
	bool m_holdingRight;
	bool m_holdingLeft;
	bool m_isPlatformer;
	bool m_slopeRelated;
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