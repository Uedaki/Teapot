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

	private:
		bool isRunning = false;

		GLFWwindow *win;

		SceneView scene;
		ImguiWrapper imgui;
		ctm::VkCore vCore;
	};
}