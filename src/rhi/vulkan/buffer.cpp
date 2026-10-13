#include "buffer.h"

#include <utility>
#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>

namespace iinf::rhi::vulkan {

VulkanBuffer::VulkanBuffer()
    : device_(nullptr), allocator_(nullptr),
    handle_(VK_NULL_HANDLE), allocation_(VK_NULL_HANDLE), allocation_info_{},
    size_(0), usage_(BufferUsage::Undefined) {
}

VulkanBuffer::VulkanBuffer(
    VkDevice device,
    VmaAllocator allocator,
    VkBuffer handle,
    VmaAllocation allocation,
    VmaAllocationInfo allocation_info,
    uint32 size,
    BufferUsage usage
) : device_(device), allocator_(allocator),
    handle_(handle), allocation_(allocation), allocation_info_(allocation_info),
    size_(size), usage_(usage) {
}

VulkanBuffer::~VulkanBuffer() {
    vmaDestroyBuffer(allocator_, handle_, allocation_);
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept : VulkanBuffer() {
    std::swap(device_, other.device_);
    std::swap(allocator_, other.allocator_);
    std::swap(handle_, other.handle_);
    std::swap(allocation_, other.allocation_);
    std::swap(allocation_info_, other.allocation_info_);
    std::swap(size_, other.size_);
    std::swap(usage_, other.usage_);
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(allocator_, other.allocator_);
    std::swap(handle_, other.handle_);
    std::swap(allocation_, other.allocation_);
    std::swap(allocation_info_, other.allocation_info_);
    std::swap(size_, other.size_);
    std::swap(usage_, other.usage_);
    return *this;
}

void* VulkanBuffer::Map() {
    if (allocation_info_.pMappedData) {
        return allocation_info_.pMappedData;
    }
    void* data = nullptr;
    vmaMapMemory(allocator_, allocation_, &data);
    return data;
}

void VulkanBuffer::Unmap() {
    if (allocation_info_.pMappedData) {
        return;
    }
    vmaUnmapMemory(allocator_, allocation_);
}

} // namespace iinf::rhi::vulkan
