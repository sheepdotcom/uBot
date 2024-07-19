#pragma once

using namespace geode::prelude;

struct playerData {
	float xPos;
	float yPos;
};

struct macroData {
	bool isPlayer1;
	int button;
	int frame;
	bool holding;
	playerData pData;
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
	UnableToOpenFile
};

class uwuBot {
public:
	static inline uwuBot* catgirl = nullptr;

	CCLabelBMFont* frameLabel = nullptr;
	CCLabelBMFont* stateLabel = nullptr;

	state m_state = off;
	std::vector<macroData> m_macroData;
	std::string m_formatVersion = "1"; //Version of the format, most likely older versions of the mod either wont accept newer formats or may give a popup warning about trying to parse a newer version, while newer versions may accept older format versions.
	std::string m_gdVersion = "2.206";
	int m_currentAction;

	int getCurrentFrame();
	void recordInput(bool isPlayer1, int button, int frame, bool holding, playerData playerData);
	BotFileError saveMacro(std::string name);
	BotFileError loadMacro(std::string name);
	void clearState();
	void updateLabels();
};