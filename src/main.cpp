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
		std::vector<bool> m_latestButtons = {false, false, false, false, false, false}; //Vector  (p1 is first 3, p2 is last 3)
		bool m_justPaused = false;
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
			//Button "fix" and feature
			for (size_t i = 0; i < catgirlsPlay->m_fields->m_latestButtons.size(); i++) {
				if (i > 2 && !catgirlsPlay->m_gameState.m_isDualMode) break; //If no second player then dont do second player stuff
				auto player = catgirlsPlay->m_player1;
				if (i > 2) player = catgirlsPlay->m_player2;
				auto button = (player->m_holdingButtons[i%3]);
				if (button != catgirlsPlay->m_fields->m_latestButtons[i]) catgirlsPlay->handleButton(button, i%3, (i > 2));
			}
			geode::log::debug("checkpoint loaded");
		}

		auto frame = uwuBot::catgirl->getCurrentFrame();
		if (uwuBot::catgirl->m_state == state::playing) {
			auto nya = std::min(static_cast<int>(uwuBot::catgirl->m_macroData.size() - 1), uwuBot::catgirl->m_currentAction);
			for (int i = nya; i >= 0; i--) {
				if (uwuBot::catgirl->m_macroData[i].frame < frame) {
					uwuBot::catgirl->m_currentAction = i;
					break;
				}
			}
		}

		uwuBot::catgirl->clearInputsAfterFrame(uwuBot::catgirl->getCurrentFrame());
	}

	void pauseGame(bool p0) {
		auto catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());
		catgirlsPlay->m_fields->m_justPaused = true;
		PlayLayer::pauseGame(p0);
		catgirlsPlay->m_fields->m_justPaused = false;
	}

	void resume() {
		auto catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());
		catgirlsPlay->m_fields->m_justPaused = true;
		PlayLayer::resume();
		catgirlsPlay->m_fields->m_justPaused = false;
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

	/*void releaseAllButtons() {
		auto catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());
		//Do this instead so I dont have to patch PlayLayer::pauseGame
		if (!catgirlsPlay->m_fields->m_justPaused) PlayerObject::releaseAllButtons();
		geode::log::debug("release buttons {}", catgirlsPlay->m_fields->m_justPaused);
	}*/
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
		//geode::log::debug("{} {} {}", holding, button, player1);
		//Update latest button list :3
		for (PlayerButtonCommand button : GJBaseGameLayer::get()->m_queuedButtons) {
			auto btn = static_cast<size_t>(button.m_button);
			auto i = btn + (button.m_isPlayer2 ? 3 : 0);
			auto catgirlsPlay = static_cast<CatgirlsPlay*>(CatgirlsPlay::get());
			catgirlsPlay->m_fields->m_latestButtons[i] = button.m_isPush;
		}

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
		//if (GJBaseGameLayer::get()) for (PlayerButtonCommand btn : GJBaseGameLayer::get()->m_queuedButtons) geode::log::debug("btn {} push {} p2 {} step {}", static_cast<int>(btn.m_button), btn.m_isPush, btn.m_isPlayer2, btn.m_step);
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
		float audioSpeed = 1.f;
		if (uwuBot::catgirl->m_state != state::off) {
			if (Mod::get()->getSettingValue<bool>("lock-delta")) {
				dt2 = 1.f / 240.f;
			}

			if (CCScheduler::get()->getTimeScale() != 1.f) {
				geode::log::debug("{}", CCScheduler::get()->getTimeScale());
			}

			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << static_cast<float>(Mod::get()->getSettingValue<double>("speedhack"));
			float speed = std::stof(ss.str());

			if (Mod::get()->getSettingValue<bool>("enable-speedhack")) {
				dt2 *= speed;
				if (Mod::get()->getSettingValue<bool>("speedhack-audio")) {
					audioSpeed *= speed;
				}
			}
		}
		FMOD::ChannelGroup* channel;
		FMODAudioEngine::sharedEngine()->m_system->getMasterChannelGroup(&channel);
		channel->setPitch(audioSpeed);
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