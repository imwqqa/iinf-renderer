#pragma once

#include <span>
#include <utility>
#include <vector>

#include <iinf/core/types.h>
#include <vulkan/vulkan_core.h>

#include "frame_context.h"
#include "texture.h"

namespace iinf::rhi::vulkan {

class VulkanSwapchain {
public:
    VulkanSwapchain();
    VulkanSwapchain(
        VkDevice device,
        VkSwapchainKHR handle,
        VkExtent2D extent,
        std::vector<VulkanTexture>&& textures,
        std::vector<VulkanFrameContext>&& frame_contexts
    );
    ~VulkanSwapchain();

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&& other) noexcept;

    VkDevice device() const { return device_; }
    VkSwapchainKHR handle() const { return handle_; }
    std::vector<VulkanTexture>& textures() { return textures_; }
    std::vector<VulkanFrameContext>& frame_contexts() { return frame_contexts_; }
    VulkanFrameContext& current_frame_context() { return frame_contexts_[inflight_index_]; }
    VkExtent2D extent() const { return extent_; }
    uint32 inflight_frames() const { return inflight_frames_; }
    uint32 inflight_index() const { return inflight_index_; }
    uint32 swapchain_image_index() const { return swapchain_image_index_; }
    void set_device(VkDevice device) { device_ = device; }
    void set_handle(VkSwapchainKHR handle) { handle_ = handle; }
    void set_textures(std::vector<VulkanTexture>&& textures) { textures_ = std::move(textures); }
    void set_frame_contexts(std::vector<VulkanFrameContext>&& frame_contexts) { frame_contexts_ = std::move(frame_contexts); }
    void set_extent(VkExtent2D extent) { extent_ = extent; }
    void set_inflight_frames(uint32 frames) { inflight_frames_ = frames; }
    void set_inflight_index(uint32 index) { inflight_index_ = index; }
    void set_swapchain_image_index(uint32 index) { swapchain_image_index_ = index; }

private:
    VkDevice device_;
    VkSwapchainKHR handle_;
    VkExtent2D extent_;
    std::vector<VulkanTexture> textures_;
    std::vector<VulkanFrameContext> frame_contexts_;
    uint32 inflight_frames_;
    uint32 inflight_index_;
    uint32 swapchain_image_index_;
};

} // namespace iinf::rhi::vulkan
