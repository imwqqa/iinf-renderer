#include "sampler.h"

#include <utility>

#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

VulkanSampler::VulkanSampler(): device_(nullptr), handle_(VK_NULL_HANDLE) {
}

VulkanSampler::VulkanSampler(VkDevice device, VkSampler handle)
: device_(device), handle_(handle) {
}

VulkanSampler::~VulkanSampler() {
    vkDestroySampler(device_, handle_, nullptr);
}

VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept : VulkanSampler() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
}

VulkanSampler& VulkanSampler::operator=(VulkanSampler&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    return *this;
}

} // namespace iinf::rhi::vulkan
