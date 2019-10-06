#include <iostream>

#include "Application.h"
#include "Teapot.h"

int main()
{
	std::cout << TEAPOT_PROJECT_NAME << " - version " << TEAPOT_VERSION_MAJOR << "." << TEAPOT_VERSION_MINOR << std::endl;

	Application app;
	return (app.run());
}