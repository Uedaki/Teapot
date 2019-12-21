#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <array>

namespace teapot
{
	namespace vk
	{
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 texCoord;
			glm::vec3 select;

			static VkVertexInputBindingDescription getBindingDescription();
			static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
		};

		struct Indice
		{
			uint32_t a;
			uint32_t b;
			uint32_t c;
		};

		struct EdgeIndice
		{
			uint32_t a;
			uint32_t b;
		};
	}
}