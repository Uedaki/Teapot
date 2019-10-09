#include "WindowManager.h"

#include <stdexcept>

void WindowManager::runWindow()
{
	initGlfw();
	vulkan.init(win);
	imgui.init(win, vulkan);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();

		// rebuild swapchain

		imgui.newFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		imgui.render(vulkan);
		memcpy(&imgui.wd.ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
	}

	imgui.destroy();
	vulkan.destroy();
	destroyGlfw();
}

void WindowManager::initGlfw()
{
	if (glfwInit() != GL_TRUE)
		throw std::runtime_error("Failed to initialized glfw");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if (!(win = glfwCreateWindow(700, 500, "Teapot", nullptr, nullptr)))
		throw std::runtime_error("Failed to create GLFWwindow");
	if (!glfwVulkanSupported())
		throw std::runtime_error("Vulkan is not supported on this device");
}

void WindowManager::destroyGlfw()
{
	glfwDestroyWindow(win);
	glfwTerminate();
}