#include "descriptor_set_layout.h"

#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device_, handle_, nullptr);
}

} // namespace iinf::rhi::vulkan
