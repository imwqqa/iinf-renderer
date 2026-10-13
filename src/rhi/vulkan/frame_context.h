#pragma once

#include "command_buffer.h"
#include "texture.h"
#include <iinf/rhi/frame_context.h>

#include <vulkan/vulkan_core.h>

#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi::vulkan {

class VulkanFrameContext final : public FrameContext {
public:
    VulkanFrameContext();
    ~VulkanFrameContext() override;

    VulkanFrameContext(const VulkanFrameContext&) = delete;
    VulkanFrameContext& operator=(const VulkanFrameContext&) = delete;
    VulkanFrameContext(VulkanFrameContext&& other) noexcept;

    VulkanCommandBuffer& command_buffer() override { return command_buffer_; }
    VulkanTexture& present_texture() override { return *present_texture_; }

    void set_command_buffer(VulkanCommandBuffer& command_buffer) { command_buffer_ = command_buffer; } 
    void set_present_texture(VulkanTexture& present_texture) { present_texture_ = &present_texture; } 

    VkDevice device() const { return device_; }
    VkCommandPool command_pool() const { return command_pool_; }
    VkSemaphore image_available_semaphore() const { return image_available_semaphore_; }
    VkSemaphore render_finished_semaphore() const { return render_finished_semaphore_; }
    VkFence render_finished_fence() const { return render_finished_fence_; }
    void set_device(VkDevice device) { device_ = device; }
    void set_command_pool(VkCommandPool command_pool) { command_pool_ = command_pool; }
    void set_image_available_semaphore(VkSemaphore semaphore) { image_available_semaphore_ = semaphore; }
    void set_render_finished_semaphore(VkSemaphore semaphore) { render_finished_semaphore_ = semaphore; }
    void set_render_finished_fence(VkFence fence) { render_finished_fence_ = fence; }

private:
    VulkanCommandBuffer command_buffer_;
    VulkanTexture* present_texture_;

    VkDevice device_;
    VkCommandPool command_pool_;
    VkSemaphore image_available_semaphore_;
    VkSemaphore render_finished_semaphore_;
    VkFence render_finished_fence_;
};

} // namespace iinf::rhi::vulkan

