#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "bot.hpp"
#include "UI/Android.hpp"
#include "UI/ImGui.hpp"

using namespace geode::prelude;

class $modify(CatgirlsPlay, PlayLayer) {
	struct Fields {
		std::unordered_map<CheckpointObject*, CheckpointSave> m_checkpoints;
	};

	bool init(GJGameLevel * level, bool useReplay, bool dontCreateObjects) {
		auto winSize = CCDirector::sharedDirector()->getWinSize();
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		uwuBot::catgirl->updateLabels();

		return true;
	}

	void resetLevel() {
		if (m_checkpointArray->count() <= 0) m_fields->m_checkpoints.clear();

		PlayLayer::resetLevel();
		uwuBot::catgirl->updateLabels();

		if (m_checkpointArray->count() > 0) return;
		uwuBot::catgirl->m_currentAction = 0;

		if (uwuBot::catgirl->m_state != state::recording) return;
		if (!uwuBot::catgirl->m_macroData.empty()) uwuBot::catgirl->m_macroData.clear();
	}

	void loadFromCheckpoint(CheckpointObject* checkpoint) {
		CatgirlsPlay* catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());

		PlayLayer::loadFromCheckpoint(checkpoint);
		if (Mod::get()->getSettingValue<bool>("practice-fix") && catgirlsPlay->m_fields->m_checkpoints.contains(checkpoint)) {
			CheckpointSave& save = catgirlsPlay->m_fields->m_checkpoints[checkpoint];
			save.apply(catgirlsPlay->m_player1, catgirlsPlay->m_gameState.m_isDualMode ? catgirlsPlay->m_player2 : nullptr);
		}

		uwuBot::catgirl->clearInputsAfterFrame(uwuBot::catgirl->getCurrentFrame());
	}

	void onQuit() {
		m_fields->m_checkpoints.clear();
		PlayLayer::onQuit();
	}

	void levelComplete() {
		PlayLayer::levelComplete();
		uwuBot::catgirl->clearState();
	}
};

class $modify(CheckpointObject) {
	bool init() {
		auto res = CheckpointObject::init();
		if (Mod::get()->getSettingValue<bool>("practice-fix")) {
			CatgirlsPlay* catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());
			if (uwuBot::catgirl->getCurrentFrame() > 0) {
				CheckpointSave save(catgirlsPlay->m_player1, catgirlsPlay->m_gameState.m_isDualMode ? catgirlsPlay->m_player2 : nullptr);
				catgirlsPlay->m_fields->m_checkpoints[this] = save;
			}
		}
		return res;
	}
};

class $modify(PlayerObject) {
	void playerDestroyed(bool p0) {
		PlayerObject::playerDestroyed(p0);
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

	void onQuit(CCObject * sender) {
		PauseLayer::onQuit(sender);
		uwuBot::catgirl->clearState();
	}

	void goEdit() {
		PauseLayer::goEdit();
		uwuBot::catgirl->clearState();
	}
};

class $modify(GJBaseGameLayer) {
	void handleButton(bool holding, int button, bool player1) {
		GJBaseGameLayer::handleButton(holding, button, player1);
		if (uwuBot::catgirl->m_state == state::recording) {
			playerData pData;
			auto player = (player1) ? this->m_player1 : this->m_player2; //Saves some code :3
			auto isFrameFix = Mod::get()->getSettingValue<bool>("frame-fix");
			if (isFrameFix) {
				pData = {
					player->getPositionX(),
					player->getPositionY(),
					player->m_platformerXVelocity,
					player->m_yVelocity
				};
			}
			else pData = { 0.f, 0.f, 0.0, 0.0 }; //No data :3
			uwuBot::catgirl->recordInput(player1, button, uwuBot::catgirl->getCurrentFrame(), holding, pData);
			uwuBot::catgirl->updateInfo(isFrameFix, isFrameFix, isFrameFix, this->m_isPlatformer);
		}
		else if (uwuBot::catgirl->m_state == state::playing) {

		}
	}

	void update(float dt) {
		if (uwuBot::catgirl->frameLabel != nullptr) {
			uwuBot::catgirl->frameLabel->setString(fmt::format("Frame: {}", uwuBot::catgirl->getCurrentFrame()).c_str());
		}
		GJBaseGameLayer::update(dt);
	}

	void processCommands(float p0) {
		GJBaseGameLayer::processCommands(p0);
		if (uwuBot::catgirl->m_state == state::playing) {
			int frame = uwuBot::catgirl->getCurrentFrame();
			if (!GJBaseGameLayer::get()->m_player1->m_isDead) {
				while (uwuBot::catgirl->m_currentAction < static_cast<int>(uwuBot::catgirl->m_macroData.size()) && frame >= uwuBot::catgirl->m_macroData[uwuBot::catgirl->m_currentAction].frame && !this->m_player1->m_isDead) {
					macroData data = uwuBot::catgirl->m_macroData[uwuBot::catgirl->m_currentAction];

					auto player = (data.isPlayer1) ? GJBaseGameLayer::get()->m_player1 : GJBaseGameLayer::get()->m_player2;
					if (data.frame == frame) {
						this->handleButton(data.holding, data.button, data.isPlayer1);

						if (uwuBot::catgirl->m_infoData.posFix && data.pData.xPos != 0 && data.pData.yPos != 0)
							player->setPosition(ccp(data.pData.xPos, data.pData.yPos));
						if (uwuBot::catgirl->m_infoData.xVelFix && uwuBot::catgirl->m_infoData.platformer && data.pData.xVel != 0)
							player->m_platformerXVelocity = data.pData.xVel;
						if (uwuBot::catgirl->m_infoData.yVelFix && data.pData.yVel != 0)
							player->m_yVelocity = data.pData.yVel;
					}

					uwuBot::catgirl->m_currentAction++;
				}
			}
		}
	}
};

class $modify(EndLevelLayer) {
	void goEdit() {
		EndLevelLayer::goEdit();
		uwuBot::catgirl->clearState();
	}

	void onMenu(CCObject * sender) {
		EndLevelLayer::onMenu(sender);
		uwuBot::catgirl->clearState();
	}
};

class $modify(CCScheduler) {
	void update(float dt) {
		//Hopefully this doesnt crash :3
		CCArray* nodes = CCDirector::sharedDirector()->getRunningScene()->getChildren();
		CCObject* obj;
		CCARRAY_FOREACH(nodes, obj) {
			if (auto popup = dynamic_cast<MacroPopup*>(obj)) {
				popup->refresh();
				break;
			}
		}
		auto dt2 = dt;
		if (uwuBot::catgirl->m_state != state::off) {
			if (Mod::get()->getSettingValue<bool>("lock-delta")) {
				dt2 = 1.f / 240.f;
			}

			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << static_cast<float>(Mod::get()->getSettingValue<double>("speedhack"));
			float speed = std::stof(ss.str());

			FMOD::ChannelGroup* channel;
			FMODAudioEngine::sharedEngine()->m_system->getMasterChannelGroup(&channel);
			channel->setPitch(1.f);

			if (Mod::get()->getSettingValue<bool>("enable-speedhack")) {
				dt2 *= speed;

				if (Mod::get()->getSettingValue<bool>("speedhack-audio")) {

					channel->setPitch(speed);
				}
			}
		}
		CCScheduler::update(dt2);
	}
};

$on_mod(Loaded) {
	uwuBot::catgirl = new uwuBot();
	/*ImGuiCocos::get().setup([] {
		//Post-setup stuff
		//Theme setup goes here
		ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.f, 0.f, 0.2f);
	}).draw([] {
		UwUGui::get()->setup();
	});*/
}