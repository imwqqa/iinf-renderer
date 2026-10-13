
#include "descriptor_pool.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>

#include "descriptor_set_layout.h"

namespace iinf::rhi::vulkan {

VulkanBumpDescriptorPool::VulkanBumpDescriptorPool() 
: device_(VK_NULL_HANDLE), sets_per_pool_(0) {
}

VulkanBumpDescriptorPool::VulkanBumpDescriptorPool(
    VkDevice device,
    std::vector<DescriptorPoolRecipeItem> descriptor_recipe,
    uint32 sets_per_pool
) : device_(device), descriptor_recipe_(std::move(descriptor_recipe)), sets_per_pool_(sets_per_pool) {
}

VulkanBumpDescriptorPool::~VulkanBumpDescriptorPool() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }

    for (VkDescriptorPool pool : available_pools_) {
        vkDestroyDescriptorPool(device_, pool, nullptr);
    }
    available_pools_.clear();

    for (VkDescriptorPool pool : full_pools_) {
        vkDestroyDescriptorPool(device_, pool, nullptr);
    }
    full_pools_.clear();
}

VulkanBumpDescriptorPool::VulkanBumpDescriptorPool(VulkanBumpDescriptorPool&& other) noexcept : VulkanBumpDescriptorPool() {
    std::swap(device_, other.device_);
    std::swap(descriptor_recipe_, other.descriptor_recipe_);
    std::swap(full_pools_, other.full_pools_);
    std::swap(available_pools_, other.available_pools_);
    std::swap(sets_per_pool_, other.sets_per_pool_);
}

VulkanBumpDescriptorPool& VulkanBumpDescriptorPool::operator=(VulkanBumpDescriptorPool&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(descriptor_recipe_, other.descriptor_recipe_);
    std::swap(full_pools_, other.full_pools_);
    std::swap(available_pools_, other.available_pools_);
    std::swap(sets_per_pool_, other.sets_per_pool_);
    return *this;
}

VkDescriptorSet VulkanBumpDescriptorPool::Allocate(VulkanDescriptorSetLayout& layout) {
    VkDescriptorSet descriptor_set;

    VkDescriptorPool pool = AcquirePool();
    VkDescriptorSetLayout layout_handle = layout.handle();
    VkDescriptorSetAllocateInfo allocation_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .pNext = nullptr };
    allocation_info.descriptorPool = pool;
    allocation_info.descriptorSetCount = 1;
    allocation_info.pSetLayouts = &layout_handle;

    VkResult result = vkAllocateDescriptorSets(device_, &allocation_info, &descriptor_set);
    while (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        full_pools_.push_back(pool);
        pool = AcquirePool();
        allocation_info.descriptorPool = pool;
        result = vkAllocateDescriptorSets(device_, &allocation_info, &descriptor_set);
    }
    available_pools_.push_back(pool);

    return descriptor_set;
}

void VulkanBumpDescriptorPool::Clear() {
    for (VkDescriptorPool pool : available_pools_) {
        vkResetDescriptorPool(device_, pool, 0);
    }

    for (VkDescriptorPool pool : full_pools_) {
        vkResetDescriptorPool(device_, pool, 0);
        available_pools_.push_back(pool);
    }
    full_pools_.clear();
}

VkDescriptorPool VulkanBumpDescriptorPool::AcquirePool() {
    VkDescriptorPool pool;

    if (available_pools_.size()) {
        pool = available_pools_.back();
        available_pools_.pop_back();
    } else {
        sets_per_pool_ = static_cast<uint32>(sets_per_pool_ * 1.5);
        sets_per_pool_ = std::min<uint32>(sets_per_pool_, 4096);
        pool = CreatePool();
    }

    return pool;
}

VkDescriptorPool VulkanBumpDescriptorPool::CreatePool() {
    VkDescriptorPool pool;

    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const DescriptorPoolRecipeItem& recipe_item : descriptor_recipe_) {
        VkDescriptorPoolSize size = {};
        size.type = recipe_item.type;
        size.descriptorCount = static_cast<uint32>(recipe_item.ratio) * 5 * (sets_per_pool_ / 8); // 5 = amount certain type of descriptor per group
        pool_sizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo pool_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .flags = 0 }; // descriptors dont free seperately from descriptor set
    pool_info.maxSets = sets_per_pool_; // 10 = max diffierent descriptor set of this pool could allocate
    pool_info.poolSizeCount = static_cast<uint32>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    
    vkCreateDescriptorPool(device_, &pool_info, nullptr, &pool);

    return pool;
}

} // namespace iinf::rhi::vulkan
