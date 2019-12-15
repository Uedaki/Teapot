#include "gui/SceneEditorWidget.h"

#include "Imgui.h"

#include "Application.h"

void teapot::gui::SceneEditorWidget::draw()
{
	ImVec2 size = ImGui::GetIO().DisplaySize;
	size.x -= 400;
	Application::get().getSceneEditor().updateExtent(size.x, size.y);

	void *data = &Application::get().getSceneEditor().getDescriptorSet();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Image(data, ImGui::GetContentRegionAvail());
	ImGui::End();
}