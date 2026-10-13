#pragma once

#include <iinf/rhi/pipeline_layout.h>

#include <vector>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/descriptor_set_layout.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi::vulkan {

class VulkanPipelineLayout final : public PipelineLayout {
public:
    VulkanPipelineLayout();
    VulkanPipelineLayout(VkDevice device, VkPipelineLayout handle);
    ~VulkanPipelineLayout() override;

    VulkanPipelineLayout(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout& operator=(const VulkanPipelineLayout&) = delete;
    VulkanPipelineLayout(VulkanPipelineLayout&& other) noexcept;
    VulkanPipelineLayout& operator=(VulkanPipelineLayout&& other) noexcept;

    std::vector<PushConstantRange>& constant_ranges() override { return constant_ranges_; }
    std::vector<DescriptorSetLayout*>& descriptor_set_layouts() override { return descriptor_set_layouts_; }

    VkPipelineLayout handle() const { return handle_; }
    void set_handle(VkPipelineLayout handle) { handle_ = handle; }

private:
    std::vector<PushConstantRange> constant_ranges_;
    std::vector<DescriptorSetLayout*> descriptor_set_layouts_;

    VkDevice device_;
    VkPipelineLayout handle_;
};

} // namespace iinf::rhi::vulkan
