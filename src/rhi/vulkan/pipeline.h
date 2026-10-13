#pragma once

#include "pipeline_layout.h"
#include <iinf/rhi/pipeline_layout.h>
#include <vulkan/vulkan_core.h>

#include <iinf/rhi/pipeline.h>

namespace iinf::rhi::vulkan {

class VulkanPipeline final : public Pipeline {
public:
    VulkanPipeline();
    VulkanPipeline(
        VkDevice device,
        VkPipeline handle,
        PipelineType type,
        PipelineLayout& layout
    );
    ~VulkanPipeline() override;

    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&&) noexcept;

    PipelineType type() const override { return type_; }
    PipelineLayout& layout() override { return *layout_; }

    VkPipeline handle() const { return handle_; }
    VkPipelineLayout layout_handle() const { return static_cast<VulkanPipelineLayout*>(layout_)->handle(); }
    void set_handle(VkPipeline handle) { handle_ = handle; }

private:
    PipelineType type_;
    PipelineLayout* layout_;

    VkDevice device_;
    VkPipeline handle_;
};

} // namespace iinf::rhi::vulkan
