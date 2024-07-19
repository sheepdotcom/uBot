#pragma once

#include <Geode/ui/TextInput.hpp>
#include "../bot.hpp"

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
	void refresh();
};