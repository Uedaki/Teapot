#include "gui/AttributeEditorWidget.h"

#include "Imgui.h"

#include "Application.h"

void teapot::gui::AttributeEditorWidget::draw()
{
	currentObject = &Application::get().getCollection().mesh;

	ImVec2 size = ImGui::GetIO().DisplaySize;
	ImGui::SetNextWindowPos(ImVec2(size.x - 400, 0));
	ImGui::SetNextWindowSize(ImVec2(400, size.y));
	ImGui::Begin("Attribute editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	if (ImGui::CollapsingHeader("Transform", nullptr, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Location: ");
		ImGui::SameLine(100);
		ImGui::InputFloat3("##1", &location[0], "%.0f");
		ImGui::Text("Rotation: ");
		ImGui::SameLine(100);
		ImGui::InputFloat3("##2", &rotation[0], "%.0f");
		ImGui::Text("Scale: ");
		ImGui::SameLine(100);
		ImGui::InputFloat3("##3", &scale[0], "%.0f");

		currentObject->updateTransform(location, rotation, scale);
	}

	ImGui::Text("Edition mode:");
	if (ImGui::RadioButton("None", mode == 0)) // NONE
	{
		mode = 0;
		Application::get().getSceneEditor().changeOverlay(mode);
	}
	if (ImGui::RadioButton("Edge", mode == 1)) // EDGE
	{
		mode = 1;
		Application::get().getSceneEditor().changeOverlay(mode);
	}
	if (ImGui::RadioButton("Vertex", mode == 2)) // VERTEX
	{
		mode = 2;
		Application::get().getSceneEditor().changeOverlay(mode);
	}

	ImGui::End();
}