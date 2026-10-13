#include "texture.h"
#include "helpers.h"

#include <utility>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi::vulkan {

VulkanTexture::VulkanTexture()
    : device_(nullptr), allocator_(nullptr),
    handle_(VK_NULL_HANDLE), view_(VK_NULL_HANDLE), allocation_(VK_NULL_HANDLE), allocation_info_(),
    extent_{}, format_(Format::Undefined), mip_levels_(0), array_layers_(0),
    usage_(TextureUsage::Undefined), owner_(TextureOwner::Undefined), layout_(VK_IMAGE_LAYOUT_UNDEFINED) {
}

VulkanTexture::VulkanTexture(
    VkImage image,
    VkImageView view,
    Extent2D extent,
    Format format,
    TextureUsage usage
) : handle_(image), view_(view), 
    extent_(extent), format_(format), mip_levels_(1), array_layers_(1),
    usage_(usage), owner_(TextureOwner::PresentEngine), layout_(VK_IMAGE_LAYOUT_UNDEFINED) { 
}

VulkanTexture::VulkanTexture(
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
) : device_(device), allocator_(allocator),
    handle_(image), view_(view), allocation_(allocation), allocation_info_(allocation_info),
    extent_(extent), format_(format), mip_levels_(mip_levels), array_layers_(array_layers),
    usage_(usage), owner_(TextureOwner::MemoryAllocator), layout_(VK_IMAGE_LAYOUT_UNDEFINED) {
}

VulkanTexture::~VulkanTexture() {
    if (owner_ == TextureOwner::MemoryAllocator) {
        vkDestroyImageView(device_, view_, nullptr);
        for (std::pair<const uint32, VkImageView>& sub_view : sub_views_) {
            vkDestroyImageView(device_, sub_view.second, nullptr);
        }
        vmaDestroyImage(allocator_, handle_, allocation_);
    }
}

VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept : VulkanTexture() {
    std::swap(device_, other.device_);
    std::swap(allocator_, other.allocator_);
    std::swap(handle_, other.handle_);
    std::swap(view_, other.view_);
    std::swap(sub_views_, other.sub_views_);
    std::swap(allocation_, other.allocation_);
    std::swap(allocation_info_, other.allocation_info_);
    std::swap(extent_, other.extent_);
    std::swap(mip_levels_, other.mip_levels_);
    std::swap(array_layers_, other.array_layers_);
    std::swap(format_, other.format_);
    std::swap(usage_, other.usage_);
    std::swap(owner_, other.owner_);
    std::swap(layout_, other.layout_);
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(allocator_, other.allocator_);
    std::swap(handle_, other.handle_);
    std::swap(view_, other.view_);
    std::swap(sub_views_, other.sub_views_);
    std::swap(allocation_, other.allocation_);
    std::swap(allocation_info_, other.allocation_info_);
    std::swap(extent_, other.extent_);
    std::swap(mip_levels_, other.mip_levels_);
    std::swap(array_layers_, other.array_layers_);
    std::swap(format_, other.format_);
    std::swap(usage_, other.usage_);
    std::swap(owner_, other.owner_);
    std::swap(layout_, other.layout_);
    return *this;
}

VkImageView VulkanTexture::view(uint32 mip_level, uint32 base_array_layer, uint32 layer_count) {
    if (mip_level == 0 && base_array_layer == 0 && layer_count == array_layers_) {
        return view_;
    }

    uint32 key = mip_level << 16 | base_array_layer << 8 | layer_count;
    if (sub_views_.contains(key)) {
        return sub_views_[key];
    }

    VkImageViewCreateInfo view_info = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    view_info.image = handle_;
    view_info.viewType = array_layers_ > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = convert::Format(format_);
    view_info.subresourceRange.aspectMask = extract::ImageAspect(format_);
    view_info.subresourceRange.baseMipLevel = mip_level;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = base_array_layer;
    view_info.subresourceRange.layerCount = layer_count;

    VkImageView view;
    vkCreateImageView(device_, &view_info, nullptr, &view);

    sub_views_[key] = view;

    return view;
}

} // namespace iinf::rhi::vulkan
