#pragma once

#include <iinf/rhi/texture.h>

#include <unordered_map>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

#include "helpers.h"

namespace iinf::rhi::vulkan {

enum class TextureOwner : uint8 {
    Undefined = 0,
    PresentEngine,
    MemoryAllocator
};

class VulkanTexture final : public Texture {
public:
    VulkanTexture();
    VulkanTexture(
        VkImage image,
        VkImageView view,
        Extent2D extent,
        Format format,
        TextureUsage usage
    ); 
    VulkanTexture(
        VkDevice device,
        VmaAllocator allocator,
        VkImage image,
        VkImageView view,
        VmaAllocation allocation,
        VmaAllocationInfo allocation_info,
        Extent2D extent,
        Format format,
        uint32 mip_levels,
        uint32 array_layers,
        TextureUsage usage
    );
    ~VulkanTexture() override;

    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(VulkanTexture&& other) noexcept;

    TextureUsage usage() const override { return usage_; }
    TextureLayout layout() const override { return convert::ImageLayout(layout_); }
    Extent2D extent() const override { return extent_; }
    Format format() const override { return format_; }
    uint32 mip_levels() const override { return mip_levels_; }
    uint32 array_layers() const override { return array_layers_; }

    VmaAllocator allocator() const { return allocator_; }
    VkImage handle() const { return handle_; }
    VkImageView view() const { return view_; }
    VkImageView view(uint32 mip_level, uint32 base_array_layer, uint32 layer_count = 1);
    VmaAllocation allocation() const { return allocation_; }
    VmaAllocationInfo allocation_info() const { return allocation_info_; }
    void set_allocator(VmaAllocator allocator) { allocator_ = allocator; }
    void set_handle(VkImage image) { handle_ = image; }
    void set_view(VkImageView view) { view_ = view; }
    void set_allocation(VmaAllocation allocation) { allocation_ = allocation; }
    void set_allocation_info(VmaAllocationInfo allocation_info) { allocation_info_ = allocation_info; }

    void set_layout(VkImageLayout layout) { layout_ = layout; }

private:
    TextureUsage usage_;
    Extent2D extent_;
    Format format_;
    uint32 mip_levels_;
    uint32 array_layers_;

    VkDevice device_;
    VmaAllocator allocator_;
    VkImage handle_;    
    VkImageView view_;
    std::unordered_map<uint32, VkImageView> sub_views_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
    TextureOwner owner_;
    VkImageLayout layout_;
};

} // namespace iinf::rhi::vulkan
