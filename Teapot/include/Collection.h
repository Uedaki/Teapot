#pragma once

#include "vulkan/Mesh.h"

namespace teapot
{
	class Collection
	{
	public:
		void init();
		void destroy();

		void unselect(DisplayMode mode);

		vk::Mesh mesh;

	private:
	};
}