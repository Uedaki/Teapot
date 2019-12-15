#include "Vulkan/Utils.h"

#include <fstream>

#include "Application.h"

uint32_t teapot::vk::Utils::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Application::get().getVulkan().physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return (i);
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void teapot::vk::Utils::createBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(Application::get().getVulkan().device, &bufferInfo, nullptr, &buffer))

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Application::get().getVulkan().device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    VK_CHECK_RESULT(vkAllocateMemory(Application::get().getVulkan().device, &allocInfo, nullptr, &bufferMemory))

    vkBindBufferMemory(Application::get().getVulkan().device, buffer, bufferMemory, 0);
}

VkShaderModule teapot::vk::Utils::createShaderModule(const std::string &shader)
{
    std::ifstream file(shader, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> code(fileSize);

    file.seekg(0);
    file.read(code.data(), fileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(Application::get().getVulkan().device, &createInfo, Application::get().getVulkan().allocator, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    return shaderModule;
}