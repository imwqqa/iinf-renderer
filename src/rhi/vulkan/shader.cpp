#include "shader.h"

#include <string>
#include <utility>

#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

VulkanShader::VulkanShader(): device_(nullptr), handle_(VK_NULL_HANDLE) {
}

VulkanShader::VulkanShader(
    VkDevice device,
    VkShaderModule handle,
    std::string entry_point
) : device_(device), handle_(handle), entry_point_(std::move(entry_point)) {
}

VulkanShader::~VulkanShader() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }
    vkDestroyShaderModule(device_, handle_, nullptr);
}

VulkanShader::VulkanShader(VulkanShader&& other) noexcept : VulkanShader() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(entry_point_, other.entry_point_);
}

VulkanShader& VulkanShader::operator=(VulkanShader&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(entry_point_, other.entry_point_);
    return *this;
}

} // namespace iinf::rhi::vulkan
