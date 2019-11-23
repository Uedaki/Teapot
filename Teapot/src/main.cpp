#include <iostream>

#include "Application.h"
#include "Log.h"
#include "Teapot.h"

int main()
{
	LOG_MSG("%s - version %d.%d", TEAPOT_PROJECT_NAME, TEAPOT_VERSION_MAJOR, TEAPOT_VERSION_MINOR);

	try
	{
		teapot::Application app;
		return (app.run());
	}
	catch (std::exception & e)
	{
		LOG_CRITICAL(e.what());
	}
}