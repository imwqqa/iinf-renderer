#pragma once

#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>

#include "descriptor_set_layout.h"

namespace iinf::rhi::vulkan {

struct DescriptorPoolRecipeItem {
    VkDescriptorType type;
    float ratio;
};

class VulkanRenderContext;

class VulkanBumpDescriptorPool {
public:
    VulkanBumpDescriptorPool();
    VulkanBumpDescriptorPool(
        VkDevice device,
        std::vector<DescriptorPoolRecipeItem> descriptor_recipe,
        uint32 sets_per_pool
    );
    ~VulkanBumpDescriptorPool();

    VulkanBumpDescriptorPool(const VulkanBumpDescriptorPool&) = delete;
    VulkanBumpDescriptorPool& operator=(const VulkanBumpDescriptorPool&) = delete;
    VulkanBumpDescriptorPool(VulkanBumpDescriptorPool&& other) noexcept;
    VulkanBumpDescriptorPool& operator=(VulkanBumpDescriptorPool&& other) noexcept;

    VkDescriptorSet Allocate(VulkanDescriptorSetLayout& layout);
    void Clear();

private:
    VkDevice device_;
    std::vector<DescriptorPoolRecipeItem> descriptor_recipe_;
    std::vector<VkDescriptorPool> full_pools_;
    std::vector<VkDescriptorPool> available_pools_;
    uint32 sets_per_pool_;

    VkDescriptorPool AcquirePool();
    VkDescriptorPool CreatePool();
};

} // namespace iinf::rhi::vulkan
