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


	if (ImGui::CollapsingHeader("Edition mode", nullptr, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Checkbox("Enable edition mode", &editionEnable))
		{
			if (editionEnable)
				Application::get().getSceneEditor().changeOverlay(mode);
			else
				Application::get().getSceneEditor().deleteOverlay();
		}

		if (editionEnable)
		{
			ImGui::Text("Mode:");
			if (ImGui::RadioButton("Face", mode == DisplayMode::FACE)) // NONE
			{
				mode = DisplayMode::FACE;
				Application::get().getSceneEditor().changeOverlay(mode);
			}
			if (ImGui::RadioButton("Edge", mode == DisplayMode::EDGE)) // EDGE
			{
				mode = DisplayMode::EDGE;
				Application::get().getSceneEditor().changeOverlay(mode);
			}
			if (ImGui::RadioButton("Vertex", mode == DisplayMode::VERTEX)) // VERTEX
			{
				mode = DisplayMode::VERTEX;
				Application::get().getSceneEditor().changeOverlay(mode);
			}
		}
	}
	ImGui::End();
}