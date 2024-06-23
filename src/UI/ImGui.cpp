#include "ImGui.hpp"

using namespace geode::prelude;

UwUGui* UwUGui::get() {
	static auto inst = new UwUGui();
	return inst;
}

void UwUGui::setup() {
	//Since this runs like every frame I can do stuff like syncing the states.
	auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("UwUBot", nullptr, flags);

	this->recording = uwuBot::catgirl->m_state == state::recording;
	this->playing = uwuBot::catgirl->m_state == state::playing;

	const char* tests[] = { "Why", "Is", "ImGui", "Like", "This" };
	static const char* test = nullptr;

	static char buf[64];
	if (ImGui::InputText("Name", buf, 64)) {
		geode::log::debug("{}", buf);
	}

	if (ImGui::Checkbox("Record", &this->recording)) {
		this->playing = false;
		uwuBot::catgirl->m_state = (this->recording) ? state::recording : state::off;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Play", &this->playing)) {
		this->recording = false;
		uwuBot::catgirl->m_state = (this->playing) ? state::playing : state::off;
	}

	if (ImGui::Button("Save")) {
		auto error = uwuBot::catgirl->saveMacro("test");
		ImGui::OpenPopup("Alert", ImGuiPopupFlags_NoOpenOverExistingPopup|ImGuiPopupFlags_NoOpenOverItems);
	}

	ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f,0.f,0.f,0.2f);
	if (ImGui::BeginPopupModal("Alert", nullptr, flags)) {
		if (ImGui::Button("Close", ImVec2(300.f,20.f))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();

	ImGui::End();
}