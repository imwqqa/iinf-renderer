#pragma once

#include <iinf/rhi/descriptor_set.h>

#include <span>

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

class VulkanDescriptorSet final : public DescriptorSet {
public:
    VulkanDescriptorSet();
    VulkanDescriptorSet(VkDevice deivce, VkDescriptorSet handle);

    VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) noexcept;

    void Update(std::span<DescriptorWrite> writes) override;

    VkDescriptorSet handle() const { return handle_; }
    void set_handle(VkDescriptorSet handle) { handle_ = handle; }

private:
    VkDevice device_;
    VkDescriptorSet handle_;
};

} // namespace iinf::rhi::vulkan
