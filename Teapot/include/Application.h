#pragma once

#include <glfw/glfw3.h>

#include "ctm/VkCore.h"
#include "ImguiWrapper.h"
#include "SceneView.h"

namespace teapot
{
	class Application
	{
	public:
		Application();
		~Application();

		int run();
		void display();

		void resize(int width = 0, int height = 0);

	private:
		bool isRunning = false;

		GLFWwindow *win;

		SceneView scene;
		ImguiWrapper imgui;
		ctm::VkCore vCore;

		teapot::Mesh mesh;

		glm::vec3 location = {0, 0, 0};
		glm::vec3 rotation = {0, 0, 0};
		glm::vec3 scale = {1, 1, 1};
		EditMode mode;
	};
}