#include "ImGui.hpp"

using namespace geode::prelude;

UwUGui* UwUGui::get() {
	static auto inst = new UwUGui();
	return inst;
}

void UwUGui::setup() {
	ImGui::Begin("Testing");
	ImGui::Text("Hi");
	ImGui::Button("Button");
	ImGui::End();
}