#pragma once

#include <iinf/rhi/descriptor_set_layout.h>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/combined_texture_sampler.h>
#include <iinf/rhi/descriptor.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi::vulkan {

class VulkanDescriptorSetLayout final : public DescriptorSetLayout {
public:
    VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout handle) : device_(device), handle_(handle) {}
    
    ~VulkanDescriptorSetLayout();

    VkDescriptorSetLayout handle() const { return handle_; }
    void set_handle(VkDescriptorSetLayout handle) { handle_ = handle; }

private:
    VkDevice device_;
    VkDescriptorSetLayout handle_;
};

} // namespace iinf::rhi::vulkan
