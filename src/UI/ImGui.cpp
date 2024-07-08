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
		//Basically buf is the text you have inputted
	}
	std::string macroName = buf;

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
		error = uwuBot::catgirl->saveMacro(macroName); //Save macro and return an error if there was one
		ImGui::OpenPopup("Macro Saved", ImGuiPopupFlags_NoOpenOverExistingPopup|ImGuiPopupFlags_NoOpenOverItems);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		error = uwuBot::catgirl->loadMacro(macroName); //Load macro and return an error if there was one
		ImGui::OpenPopup("Macro Loaded", ImGuiPopupFlags_NoOpenOverExistingPopup|ImGuiPopupFlags_NoOpenOverItems);
	}

	if (ImGui::BeginPopupModal("Macro Saved", nullptr, flags)) {
		const char* text = "Generic Error";
		switch (error) {
		case BotFileError::GenericError: text = "Generic Error"; break;
		case BotFileError::UnableToOpenFile: text = "Macro failed to save."; break;
		case BotFileError::EmptyFileName: text = "You must name your macro before saving."; break;
		case BotFileError::InvalidFileName: text = "Invalid file name."; break;
		case BotFileError::Success: text = "Macro saved."; break;
		}
		ImGui::Text(text);
		if (ImGui::Button("Close", ImVec2(300.f,20.f))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Macro Loaded", nullptr, flags)) {
		const char* text = "Generic Error";
		switch (error) {
		case BotFileError::GenericError: text = "Generic Error"; break;
		case BotFileError::UnableToOpenFile: text = "Macro failed to load."; break;
		case BotFileError::EmptyFileName: text = "You must enter a macro name to load."; break;
		case BotFileError::InvalidFileName: text = "Macro does not exist."; break;
		case BotFileError::Success: text = "Macro loaded."; break;
		}
		ImGui::Text(text);
		if (ImGui::Button("Close", ImVec2(300.f, 20.f))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}