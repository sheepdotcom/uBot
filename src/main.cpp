#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include "bot.hpp"
#include "UI/ImGui.hpp"
#include "UI/Android.hpp"

using namespace geode::prelude;

CCLabelBMFont* frameLabel = nullptr;

class $modify(PlayerObject) {
	void playerDestroyed(bool p0) {
		uwuBot::catgirl->reset();
		PlayerObject::playerDestroyed(p0);
		geode::log::debug("player destroyed");
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

	void onQuit(CCObject* sender) {
		PauseLayer::onQuit(sender);
		uwuBot::catgirl->clearState();
	}

	void goEdit() {
		PauseLayer::goEdit();
		uwuBot::catgirl->clearState();
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

	void resetLevel() {
		PlayLayer::resetLevel();
		uwuBot::catgirl->m_currentAction = 0;
		for (size_t player = 0; player < 2; player++) {
			auto p = ((player == 0) ? this->m_player1 : this->m_player2);
			p->releaseAllButtons();
		}
		if (uwuBot::catgirl->m_state == state::recording) {
			if (this->m_isPracticeMode && uwuBot::catgirl->getCurrentFrame() >= 0) {
				int frame = uwuBot::catgirl->getCurrentFrame();
				if (!uwuBot::catgirl->m_macroData.empty()) {
					for (int i = uwuBot::catgirl->m_macroData.size() - 1; i >= 0; i--) { //reverse iterator seems wierd so im using this
						if (uwuBot::catgirl->m_macroData[i].frame >= frame) {
							uwuBot::catgirl->m_macroData.erase(uwuBot::catgirl->m_macroData.begin() + i);
						}
						else break;
					}
				}
			}
			else {
				if (!uwuBot::catgirl->m_macroData.empty()) {
					uwuBot::catgirl->m_macroData.clear();
				}
			}
		}
		geode::log::debug("resetLevel");
	}

	void levelComplete() {
		PlayLayer::levelComplete();
		uwuBot::catgirl->clearState();
	}
};

class $modify(GJBaseGameLayer) {
	void handleButton(bool holding, int button, bool player1) {
		GJBaseGameLayer::handleButton(holding, button, player1);
		if (uwuBot::catgirl->m_state == state::recording) {
			playerData pData;
			auto player = (player1) ? this->m_player1 : this->m_player2; //Saves some code :3
			pData = {
				player->getPositionX(),
				player->getPositionY()
			};
			uwuBot::catgirl->recordInput(player1, button, uwuBot::catgirl->getCurrentFrame(), holding, pData);
		}
		else if (uwuBot::catgirl->m_state == state::playing) {
			
		}
	}

	void update(float dt) {
		if (frameLabel != nullptr) {
			frameLabel->setString(fmt::format("Frame: {}", uwuBot::catgirl->getCurrentFrame()).c_str());
		}
		GJBaseGameLayer::update(dt);
	}

	void processCommands(float p0) {
		if (uwuBot::catgirl->m_state == state::playing) {
			int frame = uwuBot::catgirl->getCurrentFrame();
			if (!GJBaseGameLayer::get()->m_player1->m_isDead) {
				while (uwuBot::catgirl->m_currentAction < static_cast<int>(uwuBot::catgirl->m_macroData.size()) && frame >= uwuBot::catgirl->m_macroData[uwuBot::catgirl->m_currentAction].frame && !this->m_player1->m_isDead) {
					auto data = uwuBot::catgirl->m_macroData[uwuBot::catgirl->m_currentAction];
					
					auto player = (data.isPlayer1) ? GJBaseGameLayer::get()->m_player1 : GJBaseGameLayer::get()->m_player2;
					if (data.frame == frame) {
						if (data.holding) {
							player->pushButton(static_cast<PlayerButton>(data.button));
						}
						else {
							player->releaseButton(static_cast<PlayerButton>(data.button));
						}
					}

					uwuBot::catgirl->m_currentAction++;
				}
			}
		}

		GJBaseGameLayer::processCommands(p0);
	}
};

class $modify(EndLevelLayer) {
	void goEdit() {
		EndLevelLayer::goEdit();
		uwuBot::catgirl->clearState();
	}

	void onMenu(CCObject* sender) {
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
				geode::log::debug("how?");
				popup->refresh();
				break;
			}
		}
		//Other stuff
		float dt2 = dt;
		if (Mod::get()->getSettingValue<bool>("lock-delta")) {
			dt2 = (1.f / 240.f); //Currently no way to change fps
		}
		dt2 *= static_cast<float>(Mod::get()->getSettingValue<double>("speedhack"));
		CCScheduler::update(dt2);
	}
};

$on_mod(Loaded) {
	uwuBot::catgirl = new uwuBot();
	ImGuiCocos::get().setup([] {
		//Post-setup stuff
	}).draw([] {
		UwUGui::get()->setup();
	});
}