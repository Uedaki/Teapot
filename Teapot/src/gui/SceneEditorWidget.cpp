#include "gui/SceneEditorWidget.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Imgui.h"

#include "Application.h"
#include "Log.h"
#include "ObjectPicker.h"
#include "Vulkan/Mesh.h"

#include <iostream>

void teapot::gui::SceneEditorWidget::draw()
{
	ImVec2 size = ImGui::GetIO().DisplaySize;
	size.x -= 400;
	Application::get().getSceneEditor().updateExtent(size.x, size.y);

	handleSelection(size);
	handleCameraMovement(size);

	void *data = &Application::get().getSceneEditor().getDescriptorSet();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Image(data, ImGui::GetContentRegionAvail());
	ImGui::End();
}

void teapot::gui::SceneEditorWidget::handleCameraMovement(ImVec2 size)
{
	ImVec2 mousePos = ImGui::GetMousePos();

	if (mouseButtonIndex != -1)
	{
		const float factorX = 0.10;
		const float factorY = 0.10;

		float movX = mousePos.x - mouseLastPosition.x;
		float movY = mousePos.y - mouseLastPosition.y;

		vk::SceneEditor::SceneView &view = Application::get().getSceneEditor().getSceneView();
		glm::vec3 &pos = view.pos;
		glm::vec3 &dir = view.dir;
		glm::vec3 &up = view.up;
		glm::vec3 &right = glm::cross(dir, up);

		if (mouseButtonIndex == 0)
		{
			dir	+= -movX * glm::normalize(right) * (factorX * factorX / 4)
				+ movY * glm::normalize(up) * (factorY * factorY / 4);
		}
		else if (mouseButtonIndex == 1)
		{
			pos += -glm::normalize(dir) * movY * factorY + glm::normalize(dir) * movX * factorY * factorX;
		}
		else if (mouseButtonIndex == 2)
		{
			pos += glm::normalize(right) * movX * factorX - glm::normalize(up) * movY * factorY;
		}

		Application::get().getSceneEditor().pushTransform(view);

		mouseLastPosition = mousePos;

		if (ImGui::IsMouseReleased(mouseButtonIndex) || mousePos.x >= size.x || mousePos.y >= size.y)
			mouseButtonIndex = -1;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
	{
		if (mousePos.x < size.x && mousePos.y < size.y)
		{
			for (int i = 0; i < 3; i++)
			{
				if (ImGui::IsMouseDown(i))
				{
					mouseButtonIndex = i;
					mouseLastPosition = mousePos;
					break;
				}
			}
		}
	}
}

void teapot::gui::SceneEditorWidget::handleSelection(ImVec2 size)
{
	if (ImGui::IsMouseClicked(0))
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		if (mousePos.x < size.x && mousePos.y < size.y)
		{
			glm::mat4 proj = Application::get().getSceneEditor().getSceneView().proj;
			glm::mat4 invProj = glm::inverse(proj);

			glm::mat4 view = Application::get().getSceneEditor().getSceneView().view;
			glm::mat4 invView = glm::inverse(view);

			glm::vec2 uv(static_cast<float>(mousePos.x) / size.x, static_cast<float>(mousePos.y) / size.y);
			uv = uv * 2.f - glm::vec2(1);
			glm::vec4 target = invProj * glm::vec4(uv.x, uv.y, 1, 1);
			glm::vec4 dir = invView * glm::vec4(target.x, target.y, target.z, 0);

			DisplayMode mode = Application::get().getSceneEditor().getCurrentDisplayMode();
			if (mode == DisplayMode::NONE)
			{
				ObjectPicker::Result result = ObjectPicker::findVertex(invView * glm::vec4(0, 0, 0, 1), dir, 100);
				if (result.mesh)
					; // collection selectMesh
				else
					Application::get().getCollection().unselect(mode);
			}
			else
			{
				ObjectPicker::Result result = ObjectPicker::find(mode, invView * glm::vec4(0, 0, 0, 1), dir, 100);
				if (result.mesh)
					result.mesh->select(mode, result.v1, result.v2, result.v3);
				else
					Application::get().getCollection().unselect(mode);
			}
		}
	}
}