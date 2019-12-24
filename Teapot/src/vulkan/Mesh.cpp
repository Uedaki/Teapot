#include "Vulkan/Mesh.h"

#include <glm/gtx/transform.hpp>

#include "Application.h"
#include "DisplayMode.h"
#include "vulkan/Utils.h"

void teapot::vk::Mesh::init()
{
	void *data;
	Context &vulkan = Application::get().getVulkan();

	Utils::createBuffer(vertexBuffer, vertexBufferMemory, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkMapMemory(vulkan.device, vertexBufferMemory, 0, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), 0, &data);
	memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
	vkUnmapMemory(vulkan.device, vertexBufferMemory);

	Utils::createBuffer(indiceBuffer, indiceBufferMemory, static_cast<uint32_t>(indices.size() * sizeof(Indice)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkMapMemory(vulkan.device, indiceBufferMemory, 0, static_cast<uint32_t>(indices.size() * sizeof(Indice)), 0, &data);
	memcpy(data, indices.data(), indices.size() * sizeof(Indice));
	vkUnmapMemory(vulkan.device, indiceBufferMemory);
}

void teapot::vk::Mesh::draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
	Context &vulkan = Application::get().getVulkan();

	VkDeviceSize offset = 0;
	vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
	
	vkCmdBindIndexBuffer(commandBuffer, indiceBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size() * 3), 1, 0, 0, 0);
}

void teapot::vk::Mesh::destroy()
{
	Context &vulkan = Application::get().getVulkan();

	vkDestroyBuffer(vulkan.device, vertexBuffer, vulkan.allocator);
	vkFreeMemory(vulkan.device, vertexBufferMemory, vulkan.allocator);

	vkDestroyBuffer(vulkan.device, indiceBuffer, vulkan.allocator);
	vkFreeMemory(vulkan.device, indiceBufferMemory, vulkan.allocator);
}

void teapot::vk::Mesh::updateTransform(glm::vec3 &nLocation, glm::vec3 &nRotation, glm::vec3 &nScale)
{
	if (location == nLocation && rotation == nRotation && scale == nScale)
		return;

	glm::mat4 mat(1);
	mat = glm::translate(mat, nLocation);

	mat = glm::rotate(mat, glm::radians(nRotation.x), glm::vec3(1, 0, 0));
	mat = glm::rotate(mat, glm::radians(nRotation.y), glm::vec3(0, 1, 0));
	mat = glm::rotate(mat, glm::radians(nRotation.z), glm::vec3(0, 0, 1));

	mat = glm::scale(mat, nScale);

	transform = mat;

	location = nLocation;
	rotation = nRotation;
	scale = nScale;
}

void teapot::vk::Mesh::select(DisplayMode mode, uint32_t v1, uint32_t v2, uint32_t v3)
{
	switch (mode)
	{
	case teapot::DisplayMode::FACE:
		selectFace(v1, v2, v3);
		break;
	case teapot::DisplayMode::EDGE:
		selectEdge(v1, v2);
		break;
	case teapot::DisplayMode::VERTEX:
		selectVertex(v1);
		break;
	default:
		break;
	}
}

void teapot::vk::Mesh::selectFace(uint32_t v1, uint32_t v2, uint32_t v3)
{
	vertices[v1].select.x = 1;
	vertices[v2].select.x = 1;
	vertices[v3].select.x = 1;

	Context &vulkan = Application::get().getVulkan();
	vkDeviceWaitIdle(vulkan.device);

	void *data;
	vkMapMemory(vulkan.device, vertexBufferMemory, 0, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), 0, &data);
	memcpy(data, vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));
	vkUnmapMemory(vulkan.device, vertexBufferMemory);
}

void teapot::vk::Mesh::selectEdge(uint32_t v1, uint32_t v2)
{
	vertices[v1].select.y = 1;
	vertices[v2].select.y = 1;

	Context &vulkan = Application::get().getVulkan();
	vkDeviceWaitIdle(vulkan.device);

	void *data;
	vkMapMemory(vulkan.device, vertexBufferMemory, 0, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), 0, &data);
	memcpy(data, vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));
	vkUnmapMemory(vulkan.device, vertexBufferMemory);
}

void teapot::vk::Mesh::selectVertex(uint32_t index)
{
	vertices[index].select.z = 1;

	Context &vulkan = Application::get().getVulkan();
	vkDeviceWaitIdle(vulkan.device);

	void *data;
	vkMapMemory(vulkan.device, vertexBufferMemory, index * sizeof(Vertex) + offsetof(Vertex, select), sizeof(glm::vec3), 0, &data);
	memcpy(data, &vertices[index].select, sizeof(glm::vec3));
	vkUnmapMemory(vulkan.device, vertexBufferMemory);
}

void teapot::vk::Mesh::unselect(DisplayMode mode)
{
	for (auto &vertex : vertices)
	{
		vertex.select[static_cast<uint32_t>(mode)] = 0;
	}

	Context &vulkan = Application::get().getVulkan();
	vkDeviceWaitIdle(vulkan.device);

	void *data;
	vkMapMemory(vulkan.device, vertexBufferMemory, 0, static_cast<uint32_t>(vertices.size() * sizeof(Vertex)), 0, &data);
	memcpy(data, vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));
	vkUnmapMemory(vulkan.device, vertexBufferMemory);
}