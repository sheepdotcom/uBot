#include "Android.hpp"

using namespace geode::prelude;

void loadMacro(std::string name) {
	auto error = uwuBot::catgirl->loadMacro(name);
	auto text = "Generic Error";
	switch (error) {
	case BotFileError::GenericError: text = "Generic Error"; break;
	case BotFileError::UnableToOpenFile: text = "Macro failed to load."; break;
	case BotFileError::EmptyFileName: text = "You must enter a macro name to load."; break;
	case BotFileError::InvalidFileName: text = "Macro does not exist."; break;
	case BotFileError::OlderVersion: text = "Older version of macro"; break;
	case BotFileError::Success: text = "Macro loaded."; break;
	}
	if (error != BotFileError::Success) {
		FLAlertLayer::create("Load Failed", text, "OK")->show();
	}
	else {
		FLAlertLayer::create("Macro Loaded", text, "OK")->show();
		uwuBot::catgirl->m_state = state::playing;
		uwuBot::catgirl->updateLabels();
	}
}

void MacroCell::onClick(CCObject* p0) {
	if (auto cell = static_cast<MacroCell*>(static_cast<CCNode*>(p0)->getUserData())) {
		if (cell->m_isLoad) {
			geode::createQuickPopup("Load Macro",
				fmt::format("Do you want to load \"{}\"?\nThis will clear the current macro.", cell->m_macroName),
				"No", "Yes",
				[=](auto, bool btn2) {
					loadMacro(cell->m_macroName);
				}
			);
		}
	}
}

bool MacroCell::init(std::filesystem::path path, float width, float height, bool load) {
	this->m_macroName = path.filename().replace_extension().string();
	this->m_isLoad = load;
	CCSize size = ccp(width, height);
	this->setContentSize(size);

	auto menu = CCMenu::create();
	menu->setPosition(ccp(0.f, 0.f));
	menu->setContentSize(size);
	this->addChild(menu);

	auto btnSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
	auto button = CCMenuItemSpriteExtra::create(btnSprite, this, menu_selector(MacroCell::onClick));
	button->setPosition(size/2);
	button->setContentSize(size);
	button->setOpacity(0.f);
	button->setID("cell-button");
	button->setUserData(this);
	menu->addChild(button);

	auto mainBG = CCScale9Sprite::create("GJ_square02.png", {0.f,0.f,80.f,80.f});
	mainBG->setContentSize(size);
	mainBG->setPosition(size/2);
	mainBG->setZOrder(1);
	mainBG->setID("cell-bg");
	button->addChild(mainBG);

	auto label = CCLabelBMFont::create(m_macroName.c_str(), "bigFont.fnt");
	label->limitLabelWidth(size.width-10.f, 0.5f, 0.f);
	label->setPosition(size/2);
	label->setZOrder(3);
	label->setID("cell-label");
	button->addChild(label);

	this->setID("MacroCell");

	return true;
}

MacroCell* MacroCell::create(std::filesystem::path path, float width, float height, bool load) {
	auto ret = new MacroCell();
	if (ret && ret->init(path, width, height, load)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

void refreshMacroList(ScrollLayer* scroll, bool load) {
	size_t i = 0;
	auto dir = (Mod::get()->getSaveDir().string() + "/macros");
	if (!std::filesystem::exists(dir)) std::filesystem::create_directory(dir);

	for (auto& macro : std::filesystem::directory_iterator(std::filesystem::u8path(dir))) {
		if (std::filesystem::is_regular_file(macro)) {
			auto name = macro.path().filename().string();
			if (name.ends_with(".uwu")) {
				//Lots of math so nothing is really hardcoded.
				auto edgeDist = 10.f; //Distance away from the edge of the scroll layer.
				auto gapDist = 10.f; //Distance between each macro cell.
				CCSize size = ccp((scroll->getContentWidth()/2) - edgeDist - gapDist/2, 40.f);

				auto macroCell = MacroCell::create(macro.path(), size.width, size.height, load);
				macroCell->setPosition(ccp(edgeDist + (i%2 * (size.width + gapDist)), scroll->m_contentLayer->getContentHeight() - edgeDist - (floor(i/2) * (size.height + gapDist)) - size.height));
				scroll->m_contentLayer->addChild(macroCell);

				i++;
			}
		}
	}
}

bool SaveMacroPopup::init(float mWidth, float mHeight) {
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	if (!CCLayerColor::initWithColor({ 0,0,0,105 })) return false;

	//MAIN LAYER NO WAY TRIGDHIFGRHDGIDHRIUGH%$*EITWGHE*GHRE*VHGDFIHGRIDSGI%$UHEGIDRFHSIFREHWEFHJWIS
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	this->addChild(mainLayer);
	auto bg = CCScale9Sprite::create("GJ_square02.png", {0.f,0.f,80.f,80.f});
	bg->setContentSize(ccp(mWidth, mHeight));
	bg->setPosition(winSize / 2);
	bg->setID("menu-background");
	mainLayer->addChild(bg);

	//Close button thing woah
	auto buttonMenu = CCMenu::create();
	buttonMenu->setPosition(ccp(0.f, 0.f));
	buttonMenu->setID("button-menu");
	mainLayer->addChild(buttonMenu);
	auto closeBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
		this, menu_selector(SaveMacroPopup::onClose));
	closeBtn->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + 2.5f, (winSize.height / 2) + (mHeight / 2) - 2.5f));
	closeBtn->setID("close-button");
	buttonMenu->addChild(closeBtn);

	//menu
	auto menu = CCMenu::create();
	menu->setPosition(ccp(0.f, 0.f));
	menu->setID("main-menu");
	mainLayer->addChild(menu);

	//scrolling
	auto scrollBG = CCScale9Sprite::create("square02_001.png", { 0.f,0.f,80.f,80.f });
	scrollBG->setContentSize(ccp((mWidth - 50.f), (mHeight - 60.f)));
	scrollBG->setPosition(ccp(winSize.width/2, winSize.height/2 + 15.f));
	scrollBG->setOpacity(100.f);
	scrollBG->setID("macro-scroll-bg");
	menu->addChild(scrollBG);
	auto scroll = ScrollLayer::create(scrollBG->getContentSize(), true, true);
	scroll->setContentSize(scrollBG->getContentSize());
	scroll->setPosition(winSize/2 - (scroll->getContentSize()/2) + (scrollBG->getPosition() - winSize/2));
	scroll->setID("macro-scroll-layer");
	refreshMacroList(scroll, false);
	menu->addChild(scroll);

	//Is that the save button?
	auto saveSprite = ButtonSprite::create("Save");
	saveSprite->setScale(0.75f);
	auto saveButton = CCMenuItemSpriteExtra::create(saveSprite, this, menu_selector(SaveMacroPopup::saveMacro));
	saveButton->setPosition(ccp((winSize.width / 2) + (mWidth / 2) - (saveButton->getScaledContentWidth()/2) - 10.f,
		((winSize.height / 2) - (mHeight / 2) + (scrollBG->getPositionY() - (scrollBG->getScaledContentHeight() / 2)) + 5.f) / 2));
	saveButton->setID("save-button");
	menu->addChild(saveButton);

	//Input thy name here
	m_macroNameInput = TextInput::create(100.f, "Macro Name", "bigFont.fnt");
	m_macroNameInput->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + (m_macroNameInput->getScaledContentWidth()/2) + 10.f,
		((winSize.height / 2) - (mHeight / 2) + (scrollBG->getPositionY() - (scrollBG->getScaledContentHeight() / 2)) + 5.f) / 2));
	m_macroNameInput->setID("macro-name-input");
	menu->addChild(m_macroNameInput);

	handleTouchPriority(this);
	this->setMouseEnabled(true);
	this->setKeypadEnabled(true);
	this->setTouchEnabled(true);
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -504, true);

	this->setZOrder(24);
	this->setID("SaveMacroPopup");

	return true;
}

SaveMacroPopup* SaveMacroPopup::create() {
	auto ret = new SaveMacroPopup();
	if (ret && ret->init(300.f, 200.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

void SaveMacroPopup::openPopup(CCObject* p0) {
	auto popup = SaveMacroPopup::create();

	CCDirector::sharedDirector()->getRunningScene()->addChild(popup);
}
void SaveMacroPopup::onClose(CCObject* p0) {
	this->removeFromParentAndCleanup(true);
}

void SaveMacroPopup::keyBackClicked() {
	onClose(nullptr);
}

void SaveMacroPopup::saveMacro(CCObject* p0) {
	auto error = uwuBot::catgirl->saveMacro(m_macroNameInput->getString());

	switch (error) {
	case GenericError:
		FLAlertLayer::create("Save Macro", "Generic Error", "OK")->show();
		break;
	case EmptyFileName:
		FLAlertLayer::create("Save Macro", "You must type in a macro name!", "OK")->show();
		break;
	case InvalidFileName:
		FLAlertLayer::create("Save Macro", "Invalid macro name.", "OK")->show();
		break;
	case UnableToOpenFile:
		FLAlertLayer::create("Save Macro", "Error while saving.", "OK")->show();
		break;
	case Success:
		FLAlertLayer::create("Save Macro", "Macro successfully saved!", "OK")->show();
		break;
	}
}

bool LoadMacroPopup::init(float mWidth, float mHeight) {
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	if (!CCLayerColor::initWithColor({ 0,0,0,105 })) return false;

	//Init main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	this->addChild(mainLayer);
	auto bg = CCScale9Sprite::create("GJ_square02.png", { 0.f,0.f,80.f,80.f });
	bg->setContentSize(ccp(mWidth, mHeight));
	bg->setPosition(winSize / 2);
	bg->setID("menu-background");
	mainLayer->addChild(bg);

	//Gotta close the menu
	auto buttonMenu = CCMenu::create();
	buttonMenu->setPosition(ccp(0, 0));
	buttonMenu->setID("button-menu");
	mainLayer->addChild(buttonMenu);
	auto closeBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(LoadMacroPopup::onClose));
	closeBtn->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + 2.5f, (winSize.height / 2 + (mHeight / 2) - 2.5f)));
	closeBtn->setID("close-button");
	buttonMenu->addChild(closeBtn);

	//menu no way wtf
	auto menu = CCMenu::create();
	menu->setPosition(ccp(0.f, 0.f));
	menu->setID("main-menu");
	mainLayer->addChild(menu);

	//scrolling
	auto scrollBG = CCScale9Sprite::create("square02_001.png", { 0.f,0.f,80.f,80.f });
	scrollBG->setContentSize(ccp((mWidth - 50.f), (mHeight - 60.f)));
	scrollBG->setPosition(ccp(winSize.width / 2, winSize.height / 2 + 15.f));
	scrollBG->setOpacity(100.f);
	scrollBG->setID("macro-scroll-bg");
	menu->addChild(scrollBG);
	auto scroll = ScrollLayer::create(scrollBG->getContentSize(), true, true);
	scroll->setContentSize(scrollBG->getContentSize());
	scroll->setPosition(winSize / 2 - (scroll->getContentSize() / 2) + (scrollBG->getPosition() - winSize / 2));
	scroll->setID("macro-scroll-layer");
	refreshMacroList(scroll, true);
	menu->addChild(scroll);

	//Load Button
	auto loadSprite = ButtonSprite::create("Load");
	loadSprite->setScale(0.75f);
	auto loadButton = CCMenuItemSpriteExtra::create(loadSprite, this, menu_selector(SaveMacroPopup::saveMacro));
	loadButton->setPosition(ccp((winSize.width / 2) + (mWidth / 2) - (loadButton->getScaledContentWidth() / 2) - 10.f,
		((winSize.height / 2) - (mHeight / 2) + (scrollBG->getPositionY() - (scrollBG->getScaledContentHeight() / 2)) + 5.f) / 2));
	loadButton->setID("load-button");
	menu->addChild(loadButton);

	//Input thy name here
	m_macroNameInput = TextInput::create(100.f, "Macro Name", "bigFont.fnt");
	m_macroNameInput->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + (m_macroNameInput->getScaledContentWidth() / 2) + 10.f,
		((winSize.height / 2) - (mHeight / 2) + (scrollBG->getPositionY() - (scrollBG->getScaledContentHeight() / 2)) + 5.f) / 2));
	m_macroNameInput->setID("macro-name-input");
	menu->addChild(m_macroNameInput);

	handleTouchPriority(this);
	this->setMouseEnabled(true);
	this->setKeypadEnabled(true);
	this->setTouchEnabled(true);
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -504, true);

	this->setZOrder(23);
	this->setID("LoadMacroPopup");

	return true;
}

LoadMacroPopup* LoadMacroPopup::create() {
	auto ret = new LoadMacroPopup();
	if (ret && ret->init(300.f, 200.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

void LoadMacroPopup::openPopup(CCObject* p0) {
	auto popup = LoadMacroPopup::create();

	CCDirector::sharedDirector()->getRunningScene()->addChild(popup);
}

void LoadMacroPopup::onClose(CCObject* p0) {
	this->removeFromParentAndCleanup(true);
}

void LoadMacroPopup::keyBackClicked() {
	onClose(nullptr);
}

bool MacroPopup::init(float mWidth, float mHeight) {
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	if (!CCLayerColor::initWithColor({ 0,0,0,105 })) return false;

	//Background for popup
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	this->addChild(mainLayer);
	auto bg = CCScale9Sprite::create("GJ_square02.png", { 0.f,0.f,80.f,80.f });
	bg->setContentSize(ccp(mWidth, mHeight));
	bg->setPosition(winSize / 2);
	bg->setID("menu-background");
	mainLayer->addChild(bg);

	//Button for closing the menu :3
	auto buttonMenu = CCMenu::create();
	buttonMenu->setPosition(ccp(0.f, 0.f));
	buttonMenu->setID("button-menu");
	mainLayer->addChild(buttonMenu);
	auto closeBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(MacroPopup::onClose));
	closeBtn->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + 2.5f, (winSize.height / 2) + (mHeight / 2) - 2.5f));
	closeBtn->setID("close-button");
	buttonMenu->addChild(closeBtn);

	//Buttons for macro stuff
	auto menu = CCMenu::create();
	menu->setPosition(ccp(0.f, 0.f));
	menu->setID("main-menu");
	mainLayer->addChild(menu);

	//Recording Button (Lots of stuff including things so it auto adjusts to specific changes)
	recordingToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MacroPopup::toggleRecording), 1.f);
	recordingToggle->setPosition(ccp((winSize.width / 2) - (mWidth / 2) + 35.f, (winSize.height / 2) + (mHeight / 2) - 35.f)); //I love math
	recordingToggle->setScale(0.75f);
	recordingToggle->toggle(uwuBot::catgirl->m_state == state::recording);
	recordingToggle->setID("recording-toggle");
	menu->addChild(recordingToggle);
	auto recordingLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
	recordingLabel->setAnchorPoint(ccp(0.f, 0.5f));
	recordingLabel->setPosition(recordingToggle->getPosition() + ccp((recordingToggle->getContentWidth() / 2) * recordingToggle->getScaleX() + 1.f, 0.f)); //Math 2.0
	recordingLabel->setScale(recordingToggle->getScale());
	recordingLabel->setID("recording-label");
	menu->addChild(recordingLabel);

	//Playing Button (Same story as recording button but reverse I guess)
	auto playingLabel = CCLabelBMFont::create("Playing", "bigFont.fnt");
	playingLabel->setAnchorPoint(ccp(1.f, 0.5f));
	playingLabel->setPosition(ccp((winSize.width / 2) + (mWidth / 2) - 17.5f, (winSize.height / 2) + (mHeight / 2) - 35.f));
	playingLabel->setScale(0.75f);
	playingLabel->setID("playing-label");
	playingToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MacroPopup::togglePlaying), 1.f);
	playingToggle->setPosition(playingLabel->getPosition() - ccp(playingLabel->getContentWidth() * playingLabel->getScaleX() + ((playingToggle->getContentWidth() / 2) * playingLabel->getScaleX()) + 1.f, 0.f)); //Math 3.0
	playingToggle->setScale(playingLabel->getScale());
	playingToggle->toggle(uwuBot::catgirl->m_state == state::playing);
	playingToggle->setID("playing-toggle");
	menu->addChild(playingToggle);
	menu->addChild(playingLabel); //Order switching stuff idk why im doing all this

	//Save Button?
	auto saveSprite = ButtonSprite::create("Save");
	saveSprite->setScale(0.7f);
	auto saveButton = CCMenuItemSpriteExtra::create(saveSprite, this, menu_selector(SaveMacroPopup::openPopup));
	saveButton->setPosition(ccp((winSize.width / 2) - (mWidth / 4), (recordingToggle->getPositionY()) - 30.f));
	saveButton->setID("save-button");
	menu->addChild(saveButton);

	//Load Button?
	auto loadSprite = ButtonSprite::create("Load");
	loadSprite->setScale(0.7f);
	auto loadButton = CCMenuItemSpriteExtra::create(loadSprite, this, menu_selector(LoadMacroPopup::openPopup));
	loadButton->setPosition(ccp((winSize.width / 2) + (mWidth / 4), (playingToggle->getPositionY()) - 30.f));
	loadButton->setID("load-button");
	menu->addChild(loadButton);

	auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
	settingsSpr->setScale(0.7f);
	auto settingsBtn = CCMenuItemSpriteExtra::create(settingsSpr, this, menu_selector(MacroPopup::openSettings));
	settingsBtn->setPosition(ccp((winSize.width / 2) + (mWidth / 2) - 25.f, (winSize.height / 2) - (mHeight / 2) + 25.f));
	settingsBtn->setID("settings-button");
	menu->addChild(settingsBtn);

	handleTouchPriority(this);
	this->setTouchEnabled(true);
	this->setKeypadEnabled(true);
	this->setMouseEnabled(true);
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -503, true);

	this->setZOrder(22);
	this->setID("MacroPopup");

	return true;
}

MacroPopup* MacroPopup::create() {
	auto ret = new MacroPopup();
	if (ret && ret->init(300.f, 200.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

void MacroPopup::openPopup(CCObject* p0) {
	auto popup = MacroPopup::create();

	CCDirector::get()->getRunningScene()->addChild(popup);
}

void MacroPopup::onClose(CCObject* p0) {
	this->removeFromParentAndCleanup(true);
}

void MacroPopup::keyBackClicked() {
	onClose(nullptr);
}

void MacroPopup::toggleRecording(CCObject* p0) {
	if (uwuBot::catgirl->m_state == state::playing) this->playingToggle->toggle(false);
	uwuBot::catgirl->m_state = (uwuBot::catgirl->m_state == state::recording) ? state::off : state::recording;
	uwuBot::catgirl->updateLabels();
}

void MacroPopup::togglePlaying(CCObject* p0) {
	if (uwuBot::catgirl->m_state == state::recording) this->recordingToggle->toggle(false);
	uwuBot::catgirl->m_state = (uwuBot::catgirl->m_state == state::playing) ? state::off : state::playing;
	uwuBot::catgirl->updateLabels();
}

void MacroPopup::openSettings(CCObject* p0) {
	geode::openSettingsPopup(Mod::get());
}

void MacroPopup::refresh() {
	if (this->recordingToggle) this->recordingToggle->toggle(uwuBot::catgirl->m_state == state::recording);
	if (this->playingToggle) this->playingToggle->toggle(uwuBot::catgirl->m_state == state::playing);
}