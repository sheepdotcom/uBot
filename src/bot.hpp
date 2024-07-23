#pragma once

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
	void updateInfo(bool posFix, bool yVelFix, bool xVelFix, bool platformer);
	BotFileError saveMacro(std::string name);
	BotFileError loadMacro(std::string name);
	void clearState();
	void updateLabels();
};