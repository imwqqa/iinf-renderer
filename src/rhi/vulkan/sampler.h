#pragma once

#include <iinf/rhi/sampler.h>

#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {
    
class VulkanSampler final : public Sampler {
public:
    VulkanSampler();
    VulkanSampler(VkDevice device, VkSampler handle);
    ~VulkanSampler() override;

    VulkanSampler(const VulkanSampler&) = delete;
    VulkanSampler& operator=(const VulkanSampler&) = delete;
    VulkanSampler(VulkanSampler&&) noexcept;
    VulkanSampler& operator=(VulkanSampler&&) noexcept;
    
    VkSampler handle() const { return handle_; }
    void set_handle(VkSampler handle) { handle_ = handle; }

private:
    VkDevice device_;
    VkSampler handle_;
};

} // namespace iinf::rhi::vulkan
