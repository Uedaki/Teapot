#pragma once

#include <thread>

class Application
{
public:
	Application();
	~Application();

	int run();

	void renderingLoop();

private:
	bool isRunning = false;

	std::thread renderingThread;
};