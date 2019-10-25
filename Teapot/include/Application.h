#pragma once

#include <glfw/glfw3.h>

#include "ImguiWrapper.h"
#include "ctm/VkCore.h"

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

		ImguiWrapper imgui;
		ctm::VkCore vCore;
	};
}