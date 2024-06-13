#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

int frame = 0;
CCLabelBMFont* frameLabel = nullptr;

//Some code is based off xdBot
//xdBot stores a little too much data and also doesn't compress it

struct playerData {
	float xPos;
	float yPos;
};

struct macroData {
	bool isPlayer1;
	int button; //You actually need this because platformer mode :3
	int frame;
	bool holding; //This name sounded wrong to me until I realized how stupid the name I gave it was.
	playerData playerData; //Why store both p1 and p2 data when you already have a bool for isPlayer1?
};

enum state {
	off,
	recording,
	playing
};

class uwuBot {
public:
	state state = off;
	std::vector<macroData> macroData;

	int getCurrentFrame() {
		return (PlayLayer::get() != nullptr) ? PlayLayer::get()->m_gameState.m_currentProgress : -1; //This is real? Crash fix?
	}

	void recordAction(bool isPlayer1, int button, int frame, bool holding, playerData playerData) {
		macroData.push_back({isPlayer1, button, frame, holding, playerData});
	}
};

uwuBot catgirl; //Never change this variable name, even if the mod name changes

class $modify(PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		auto winSize = CCDirector::sharedDirector()->getWinSize();
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		frameLabel = CCLabelBMFont::create("Frame: 0", "bigFont.fnt");
		frameLabel->setAnchorPoint(ccp(0.f, 1.f));
		frameLabel->setPosition(ccp(0.f, winSize.height));
		frameLabel->setID("frame-label");
		frameLabel->setScale(0.5f);
		frameLabel->setZOrder(128);

		PlayLayer::get()->addChild(frameLabel);

		return true;
	}
};

class $modify(GJBaseGameLayer) {
	void update(float dt) {
		if (frameLabel != nullptr) {
			frameLabel->setString(std::format("Frame: {}", catgirl.getCurrentFrame()).c_str());
		}
		if (catgirl.getCurrentFrame() == 90) {
			m_player1->pushButton(PlayerButton::Jump);
		}
		else if (catgirl.getCurrentFrame() == 91) {
			m_player1->releaseButton(PlayerButton::Jump);
		}
		GJBaseGameLayer::update(dt);
	}
};