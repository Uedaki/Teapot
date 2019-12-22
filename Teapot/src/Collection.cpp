#include "Collection.h"

void teapot::Collection::init()
{
	mesh.init();
}

void teapot::Collection::destroy()
{
	mesh.destroy();
}

void teapot::Collection::unselect(DisplayMode mode)
{
	mesh.unselect(mode);
}