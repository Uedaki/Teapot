#pragma once

#include <thread>

#include "WindowManager.h"

class Application
{
public:
	Application();
	~Application();

	int run();

	//void renderingLoop();

private:
	bool isRunning = false;

	WindowManager winManager;

	//std::thread renderingThread;
};