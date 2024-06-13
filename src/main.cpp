#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

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

uwuBot catgirl; //Never change this variable name, catgirl

class MacroPopup : public CCLayerColor {
protected:
	CCMenuItemToggler* recordingToggle = nullptr;
	CCMenuItemToggler* playingToggle = nullptr;

	bool init(float mWidth, float mHeight) {
		auto winSize = CCDirector::sharedDirector()->getWinSize();
		if (!CCLayerColor::initWithColor({0,0,0,105})) return false;

		//Background for popup
		auto mainLayer = CCLayer::create();
		mainLayer->setID("");
		this->addChild(mainLayer);
		auto bg = CCScale9Sprite::create("GJ_square02.png", {0.f,0.f,80.f,80.f});
		bg->setContentSize(ccp(mWidth, mHeight));
		bg->setPosition(winSize / 2);
		bg->setID("menu-background");
		mainLayer->addChild(bg);

		//Button for closing the menu :3
		auto buttonMenu = CCMenu::create();
		buttonMenu->setID("button-menu");
		mainLayer->addChild(buttonMenu);
		auto closeBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(MacroPopup::onClose));
		closeBtn->setPosition(ccp((-mWidth+26.f)/2,(mHeight-26.75f)/2));
		closeBtn->setID("close-button");
		buttonMenu->addChild(closeBtn);

		//Buttons for macro stuff
		auto menu = CCMenu::create();
		menu->setPosition(ccp(0.f, 0.f));
		menu->setID("main-menu");
		mainLayer->addChild(menu);

		//Recording Button (Lots of stuff including things so it auto adjusts to specific changes)
		recordingToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MacroPopup::toggleRecording), 1.f);
		recordingToggle->setPosition(ccp((winSize.width/2)-(mWidth/2)+35.f, (winSize.height/2)+(mHeight/2)-35.f)); //I love math
		recordingToggle->setScale(0.75f);
		recordingToggle->toggle(catgirl.state == state::recording);
		recordingToggle->setID("recording-toggle");
		menu->addChild(recordingToggle);
		auto recordingLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
		recordingLabel->setAnchorPoint(ccp(0.f,0.5f));
		recordingLabel->setPosition(recordingToggle->getPosition() + ccp((recordingToggle->getContentWidth() / 2) * recordingToggle->getScaleX() + 1.f, 0.f)); //Math 2.0
		recordingLabel->setScale(recordingToggle->getScale());
		recordingLabel->setID("recording-label");
		menu->addChild(recordingLabel);

		//Playing Button (Same story as recording button but reverse I guess)
		auto playingLabel = CCLabelBMFont::create("Playing", "bigFont.fnt");
		playingLabel->setAnchorPoint(ccp(1.f, 0.5f));
		playingLabel->setPosition(ccp((winSize.width/2)+(mWidth/2)-17.5f, (winSize.height/2)+(mHeight/2)-35.f));
		playingLabel->setScale(0.75f);
		playingLabel->setID("playing-label");
		playingToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MacroPopup::togglePlaying), 1.f);
		playingToggle->setPosition(playingLabel->getPosition() - ccp(playingLabel->getContentWidth() * playingLabel->getScaleX() + ((playingToggle->getContentWidth() / 2) * playingLabel->getScaleX()) + 1.f, 0.f)); //Math 3.0
		playingToggle->setScale(playingLabel->getScale());
		playingToggle->toggle(catgirl.state == state::playing);
		playingToggle->setID("playing-toggle");
		menu->addChild(playingToggle);
		menu->addChild(playingLabel); //Order switching stuff idk why im doing all this

		handleTouchPriority(this);
		this->setTouchEnabled(true);
		this->setKeypadEnabled(true);
		this->setMouseEnabled(true);
		CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -612, true);

		this->setZOrder(22);
		this->setID("MacroPopup");

		return true;
	}

	static MacroPopup* create() {
		auto ret = new MacroPopup();
		if (ret && ret->init(300.f, 200.f)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

public:
	void openPopup(CCObject* p0) {
		auto popup = MacroPopup::create();

		CCDirector::get()->getRunningScene()->addChild(popup);
	}

	void onClose(CCObject* p0) {
		this->removeFromParentAndCleanup(true);
	}

	void keyBackClicked() {
		onClose(nullptr);
	}

	void toggleRecording(CCObject* p0) {
		if (catgirl.state == state::playing) this->playingToggle->toggle(false);
		catgirl.state = (catgirl.state == state::recording) ? state::off : state::recording;
	}

	void togglePlaying(CCObject* p0) {
		if (catgirl.state == state::recording) this->recordingToggle->toggle(false);
		catgirl.state = (catgirl.state == state::playing) ? state::off : state::recording;
	}
};

class $modify(PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();

		auto sprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png"); //Yes this is the texture used in xdBot idc right now this is alpha
		sprite->setScale(0.35f);

		auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MacroPopup::openPopup));

		auto rightMenu = this->getChildByID("right-button-menu");
		rightMenu->addChild(btn);
		rightMenu->updateLayout();
	}
};

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
		/*if (catgirl.getCurrentFrame() == 90) {
			m_player1->pushButton(PlayerButton::Jump);
		}
		else if (catgirl.getCurrentFrame() == 91) {
			m_player1->releaseButton(PlayerButton::Jump);
		}*/
		GJBaseGameLayer::update(dt);
	}
};