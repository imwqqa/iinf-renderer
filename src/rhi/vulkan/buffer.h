#pragma once

#include <iinf/rhi/buffer.h>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <iinf/core/types.h>

namespace iinf::rhi::vulkan {

class VulkanBuffer final : public Buffer {
public:
    VulkanBuffer();
    VulkanBuffer(
        VkDevice device,
        VmaAllocator allocator,
        VkBuffer handle,
        VmaAllocation allocation,
        VmaAllocationInfo allocation_info,
        uint32 size,
        BufferUsage usage
    );
    ~VulkanBuffer() override;

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

    void* Map() override;
    void Unmap() override;

    uint32 size() const override { return size_; }
    BufferUsage usage() const override { return usage_; }

    VkBuffer handle() const { return handle_; }
    void set_handle(VkBuffer handle) { handle_ = handle; }

private:
    uint32 size_;
    BufferUsage usage_;

    VkDevice device_;
    VmaAllocator allocator_;
    VkBuffer handle_;    
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};

} // namespace iinf::rhi::vulkan
