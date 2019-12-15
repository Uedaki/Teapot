#pragma once

#include <glfw/glfw3.h>

#include "Collection.h"
#include "gui/Manager.h"
#include "vulkan/Command.h"
#include "vulkan/Context.h"
#include "vulkan/SceneEditor.h"

namespace teapot::vk { class Command; }

namespace teapot
{
	class Application
	{
	public:
		Application();
		~Application();

		inline static Application &get() { return (*app); }
		inline GLFWwindow *getWindow() { return (win); }
		inline vk::Context &getVulkan() { return (vulkan); }
		inline gui::Manager &getGui() { return (gui); }
		inline vk::SceneEditor &getSceneEditor() { return (sceneEditor); }
		inline Collection &getCollection() { return (collection); }

		void init();
		void destroy();

		int run();
		void resize(int w, int h);

	private:
		static Application *app;

		bool isInitialized = false;

		GLFWwindow *win = nullptr;

		vk::Context vulkan;
		gui::Manager gui;
		vk::SceneEditor sceneEditor;
		Collection collection;

		vk::Command command;

		void initGlfw();
		void destroyGlfw();

		void render(vk::Command &command);
	};
}