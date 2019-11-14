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

		float location[3] = {0, 0, 0};
		float rotation[3] = {0, 0, 0};
		float scale[3] = {1, 1, 1};
	};
}