#include "swapchain.h"

#include <utility>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "frame_context.h"
#include "texture.h"

namespace iinf::rhi::vulkan {

VulkanSwapchain::VulkanSwapchain() {
    device_ = VK_NULL_HANDLE;
    handle_ = VK_NULL_HANDLE;
    extent_ = { .width = 0, .height = 0 };
    inflight_frames_ = 0;
    inflight_index_ = 0;
    swapchain_image_index_ = 0;
}

VulkanSwapchain::~VulkanSwapchain() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }
    for (const VulkanTexture& texture : textures_) {
        vkDestroyImageView(device_, texture.view(), nullptr);
    }
    vkDestroySwapchainKHR(device_, handle_, nullptr);
}

VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) noexcept : VulkanSwapchain() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(extent_, other.extent_);
    std::swap(textures_, other.textures_);
    std::swap(frame_contexts_, other.frame_contexts_);
    std::swap(inflight_frames_, other.inflight_frames_);
    std::swap(inflight_index_, other.inflight_index_);
    std::swap(swapchain_image_index_, other.swapchain_image_index_);
}

VulkanSwapchain::VulkanSwapchain(
    VkDevice device,
    VkSwapchainKHR handle,
    VkExtent2D extent,
    std::vector<VulkanTexture>&& textures,
    std::vector<VulkanFrameContext>&& frame_contexts
) : device_(device), handle_(handle), extent_(extent),
    textures_(std::move(textures)), frame_contexts_(std::move(frame_contexts)) {
}

} // namespace iinf::rhi::vulkan

