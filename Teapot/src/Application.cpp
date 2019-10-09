#include "Application.h"

#include <chrono>
#include <iostream>

Application::Application()
//	: renderingThread([this]() { this->winManager.runWindow(); })
{
	isRunning = true;
}

Application::~Application()
{
	//if (renderingThread.joinable())
	//	renderingThread.join();
}

int Application::run()
{
	winManager.runWindow();
	//if (!isRunning)
	//	return (1);
	//std::chrono::milliseconds dura(2000);
	//std::this_thread::sleep_for(dura);
	//isRunning = false;
	return (0);
}