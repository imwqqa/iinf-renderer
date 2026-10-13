#include "frame_context.h"
#include <utility>
#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

VulkanFrameContext::VulkanFrameContext() {
    present_texture_ = nullptr;
    device_ = VK_NULL_HANDLE;
    command_pool_ = VK_NULL_HANDLE;
    image_available_semaphore_ = VK_NULL_HANDLE;
    render_finished_semaphore_ = VK_NULL_HANDLE;
    render_finished_fence_ = VK_NULL_HANDLE;
}

VulkanFrameContext::~VulkanFrameContext() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }
    vkDestroyCommandPool(device_, command_pool_, nullptr);
    vkDestroyFence(device_, render_finished_fence_, nullptr);
    vkDestroySemaphore(device_, render_finished_semaphore_, nullptr);
    vkDestroySemaphore(device_, image_available_semaphore_, nullptr);
}

VulkanFrameContext::VulkanFrameContext(VulkanFrameContext&& other) noexcept : VulkanFrameContext() {
    std::swap(device_, other.device_);
    std::swap(present_texture_, other.present_texture_);
    std::swap(command_pool_, other.command_pool_);
    std::swap(command_buffer_, other.command_buffer_);
    std::swap(render_finished_fence_, other.render_finished_fence_);
    std::swap(render_finished_semaphore_, other.render_finished_semaphore_);
    std::swap(image_available_semaphore_, other.image_available_semaphore_);
}

} // namespace iinf::rhi::vulkan
