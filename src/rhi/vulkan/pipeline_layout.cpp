#include "pipeline_layout.h"

#include <utility>
#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

VulkanPipelineLayout::VulkanPipelineLayout(): device_(nullptr), handle_(VK_NULL_HANDLE) {
}

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout handle)
    : device_(device), handle_(handle) {
}

VulkanPipelineLayout::~VulkanPipelineLayout() {
    vkDestroyPipelineLayout(device_, handle_, nullptr);
}

VulkanPipelineLayout::VulkanPipelineLayout(VulkanPipelineLayout&& other) noexcept : VulkanPipelineLayout() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(constant_ranges_, other.constant_ranges());
    std::swap(descriptor_set_layouts_, other.descriptor_set_layouts_);
}

VulkanPipelineLayout& VulkanPipelineLayout::operator=(VulkanPipelineLayout&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(constant_ranges_, other.constant_ranges());
    std::swap(descriptor_set_layouts_, other.descriptor_set_layouts_);
    return *this;
}

} // namespace iinf::rhi::vulkan
