#include "pipeline.h"

#include <utility>
#include <vulkan/vulkan_core.h>

#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/pipeline_layout.h>

namespace iinf::rhi::vulkan {

VulkanPipeline::VulkanPipeline()
    : device_(nullptr), handle_(VK_NULL_HANDLE),
    type_(PipelineType::Graphics), layout_(nullptr) {
}

VulkanPipeline::VulkanPipeline(
    VkDevice device,
    VkPipeline handle,
    PipelineType type,
    PipelineLayout& layout
) : device_(device), handle_(handle), type_(type), layout_(&layout) {
}

VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipeline(device_, handle_, nullptr);
}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept : VulkanPipeline() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(type_, other.type_);
    std::swap(layout_, other.layout_);
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    std::swap(type_, other.type_);
    std::swap(layout_, other.layout_);
    return *this;
}

} // namespace iinf::rhi::vulkan


