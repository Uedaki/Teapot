#include "Vulkan/Mesh.h"

#include <glm/gtx/transform.hpp>

#include "Application.h"
#include "vulkan/Utils.h"
#include "vulkan/SingleTimeCommandPool.h"

void teapot::vk::Mesh::init()
{
	void *data;
	Context &vulkan = Application::get().getVulkan();

	vertexBuffers.resize(vulkan.swapchainInfo.imgCount);
	vertexBufferMemories.resize(vulkan.swapchainInfo.imgCount);
	indiceBuffers.resize(vulkan.swapchainInfo.imgCount);
	indiceBufferMemories.resize(vulkan.swapchainInfo.imgCount);
	for (uint32_t i = 0; i < vulkan.swapchainInfo.imgCount; i++)
	{
		Utils::createBuffer(vertexBuffers[i], vertexBufferMemories[i], static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkMapMemory(vulkan.device, vertexBufferMemories[i], 0, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), 0, &data);
		memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
		vkUnmapMemory(vulkan.device, vertexBufferMemories[i]);

		Utils::createBuffer(indiceBuffers[i], indiceBufferMemories[i], static_cast<uint32_t>(indices.size() * sizeof(Indice)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkMapMemory(vulkan.device, indiceBufferMemories[i], 0, static_cast<uint32_t>(indices.size() * sizeof(Indice)), 0, &data);
		memcpy(data, indices.data(), indices.size() * sizeof(Indice));
		vkUnmapMemory(vulkan.device, indiceBufferMemories[i]);
	}
}

void teapot::vk::Mesh::draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
	Context &vulkan = Application::get().getVulkan();

	VkDeviceSize offset = 0;
	vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffers[vulkan.swapchainInfo.currImg], &offset);
	vkCmdBindIndexBuffer(commandBuffer, indiceBuffers[vulkan.swapchainInfo.currImg], 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size() * 3), 1, 0, 0, 0);
}

void teapot::vk::Mesh::destroy()
{
	Context &vulkan = Application::get().getVulkan();

	for (uint32_t i = 0; i < vertexBuffers.size(); i++)
	{
		vkDestroyBuffer(vulkan.device, vertexBuffers[i], vulkan.allocator);
		vkFreeMemory(vulkan.device, vertexBufferMemories[i], vulkan.allocator);

		vkDestroyBuffer(vulkan.device, indiceBuffers[i], vulkan.allocator);
		vkFreeMemory(vulkan.device, indiceBufferMemories[i], vulkan.allocator);
	}
}

void teapot::vk::Mesh::updateTransform(glm::vec3 &nLocation, glm::vec3 &nRotation, glm::vec3 &nScale)
{
	if (location == nLocation && rotation == nRotation && scale == nScale)
		return;

	glm::mat4 mat(1);
	mat = glm::translate(mat, nLocation);

	mat = glm::rotate(mat, nRotation.x, glm::vec3(1, 0, 0));
	mat = glm::rotate(mat, nRotation.y, glm::vec3(0, 1, 0));
	mat = glm::rotate(mat, nRotation.z, glm::vec3(0, 0, 1));

	mat = glm::scale(mat, nScale);

	transform = mat;
}