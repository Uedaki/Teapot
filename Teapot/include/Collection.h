#pragma once

#include "vulkan/Mesh.h"

namespace teapot
{
	class Collection
	{
	public:
		void init();
		void destroy();

		vk::Mesh mesh;

	private:
	};
}