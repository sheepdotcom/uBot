#pragma once

#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include "../bot.hpp"

class CCContentSizeTo : public CCActionInterval {
protected:
	CCSize m_endSize;
	CCSize m_startSize;
	CCNode* m_target;

	bool initWithDuration(float duration, const CCSize& size) {
		if (!CCActionInterval::initWithDuration(duration)) return false;
		this->m_endSize = size;
		return true;
	}

	void startWithTarget(CCNode* target) {
		CCActionInterval::startWithTarget(target);
		this->m_target = target;
		this->m_startSize = target->getContentSize();
	}

public:
	static CCContentSizeTo* create(float duration, const CCSize& size) {
		auto ret = new CCContentSizeTo();
		if (ret && ret->initWithDuration(duration, size)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	virtual void update(float time) {
		if (!this->m_target) return;
		CCSize newSize(this->m_startSize + (this->m_endSize - this->m_startSize) * time);
		this->m_target->setContentSize(newSize);
	}
};

class CCScaleToWithDelay : public CCActionInterval {
protected:
	CCSize m_endScale;
	CCSize m_startScale;
	float m_delay;
	CCNode* m_target;

	bool initWithDuration(float duration, CCSize scale, float delay) {
		if (!CCActionInterval::initWithDuration(duration + delay)) return false;
		this->m_endScale = scale;
		this->m_delay = delay / (duration + delay);
		return true;
	}

	void startWithTarget(CCNode* target) {
		CCActionInterval::startWithTarget(target);
		this->m_target = target;
		this->m_startScale = ccp(target->getScaleX(), target->getScaleY());
	}

public:
	static CCScaleToWithDelay* create(float duration, CCSize scale, float delay) {
		auto ret = new CCScaleToWithDelay();
		if (ret && ret->initWithDuration(duration, scale, delay)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	virtual void update(float time) {
		if (!this->m_target) return;
		CCSize newScale = this->m_startScale + (this->m_endScale - this->m_startScale) * (std::max(time - this->m_delay, 0.f) * (1 / (1 - this->m_delay)));
		this->m_target->setScale(newScale.width, newScale.height);
	}
};

class Dropdown : public CCNode {
public:
	CCMenu* m_menu;
	CCScale9Sprite* m_background;
	CCSprite* m_arrow;
	CCLabelBMFont* m_text;
	std::vector<std::string> m_content;
	std::vector<CCMenuItemSpriteExtra*> m_buttons;
	SEL_MenuHandler m_callback;
	int m_selected = 0;
	bool m_open = false;
	bool m_openDown = true;

protected:
	void toggle(CCObject* p0);
	void selectOption(CCObject* p0);
	bool init(CCSize size, std::vector<std::string> content, int selected = 0, SEL_MenuHandler callback = nullptr, bool openDown = true);

public:
	static Dropdown* create(CCSize size, std::vector<std::string> content, int selected = 0, SEL_MenuHandler callback = nullptr, bool openDown = true);
};

class MacroCell : public CCNode {
protected:
	std::string m_macroName;
	bool m_isLoad;

	bool init(std::filesystem::path path, float width, float height, bool load);
	void onClick(CCObject* p0);
public:
	static MacroCell* create(std::filesystem::path path, float width, float height, bool load);
};

class SaveMacroPopup : public CCLayerColor, public TextInputDelegate {
protected:
	TextInput* m_macroNameInput = nullptr;

	bool init(float mWidth, float mHeight);
	static SaveMacroPopup* create();
public:
	void openPopup(CCObject* p0);
	void onClose(CCObject* p0);
	void keyBackClicked();
	void saveMacro(CCObject* p0);
};

class LoadMacroPopup : public CCLayerColor {
protected:
	TextInput* m_macroNameInput = nullptr;

	bool init(float mWidth, float mHeight);
	static LoadMacroPopup* create();
public:
	void openPopup(CCObject* p0);
	void onClose(CCObject* p0);
	void keyBackClicked();
};

class ConvertMacroPopup : public CCLayerColor {
protected:
	bool init(float mWidth, float mHeight);
	static ConvertMacroPopup* create();
public:
	void openPopup(CCObject* p0);
	void onClose(CCObject* p0);
	void keyBackClicked();
};

class MacroPopup : public CCLayerColor {
protected:
	CCMenuItemToggler* recordingToggle = nullptr;
	CCMenuItemToggler* playingToggle = nullptr;

	bool init(float mWidth, float mHeight);
	static MacroPopup* create();
public:
	void openPopup(CCObject* p0);
	void onClose(CCObject* p0);
	void keyBackClicked();
	void toggleRecording(CCObject* p0);
	void togglePlaying(CCObject* p0);
	void openSettings(CCObject* p0);
	void refresh();
};