#pragma once

#include <glm/glm.hpp>

#include "vulkan/Mesh.h"

namespace teapot
{
	class ObjectPicker
	{
	public:
		struct Result
		{
			vk::Mesh *mesh = nullptr;
			uint32_t v1 = 0;
			uint32_t v2 = 0;
			uint32_t v3 = 0;
		};

		static inline Result find(DisplayMode mode, glm::vec3 origin, glm::vec3 dir, float maxDist = 100)
		{
			Result(*fct[3])(glm::vec3, glm::vec3, float) = {findFace, findEdge, findVertex};
			return (fct[static_cast<uint32_t>(mode)](origin, dir, maxDist));
		}

		static Result findVertex(glm::vec3 origin, glm::vec3 dir, float maxDist = 100);
		static Result findEdge(glm::vec3 origin, glm::vec3 dir, float maxDist = 100);
		static Result findFace(glm::vec3 origin, glm::vec3 dir, float maxDist = 100);
	};
}