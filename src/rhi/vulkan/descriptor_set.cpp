#include "descriptor_set.h"

#include <deque>
#include <span>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <iinf/rhi/buffer.h>
#include <iinf/rhi/combined_texture_sampler.h>
#include <iinf/rhi/descriptor.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/texture.h>

#include "buffer.h"
#include "helpers.h"
#include "texture.h"
#include "sampler.h"

namespace iinf::rhi::vulkan {

VulkanDescriptorSet::VulkanDescriptorSet(): device_(nullptr), handle_(VK_NULL_HANDLE) {
}

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice deivce, VkDescriptorSet handle)
    : device_(deivce), handle_(handle) {
}

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept : VulkanDescriptorSet() {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
}

VulkanDescriptorSet& VulkanDescriptorSet::operator=(VulkanDescriptorSet&& other) noexcept {
    std::swap(device_, other.device_);
    std::swap(handle_, other.handle_);
    return *this;
}

void VulkanDescriptorSet::Update(std::span<DescriptorWrite> writes) {
    std::vector<VkWriteDescriptorSet> vk_writes;
    std::deque<VkDescriptorImageInfo> image_infos;
    std::deque<VkDescriptorBufferInfo> buffer_infos;

    for (DescriptorWrite write : writes) {
        VkDescriptorImageInfo* image_info_ptr = nullptr;
        VkDescriptorBufferInfo* buffer_info_ptr = nullptr;

        std::visit([&](auto* descriptor) {
            using T = std::remove_pointer_t<decltype(descriptor)>;
            if constexpr (std::is_same_v<T, Buffer>) {
                VkBuffer buffer = static_cast<VulkanBuffer&>(*descriptor).handle();
                buffer_infos.push_back({ buffer, 0, VK_WHOLE_SIZE });
                buffer_info_ptr = &buffer_infos.back();
            } else if constexpr (std::is_same_v<T, Texture>) {
                VkImageView view = static_cast<VulkanTexture&>(*descriptor).view();
                VkImageLayout layout = write.type == DescriptorType::StorageTexture ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_infos.push_back({ VK_NULL_HANDLE, view, layout });
                image_info_ptr = &image_infos.back();
            } else if constexpr (std::is_same_v<T, Sampler>) {
                VkSampler sampler = static_cast<VulkanSampler&>(*descriptor).handle();
                image_infos.push_back({ sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED });
                image_info_ptr = &image_infos.back();
            } else if constexpr (std::is_same_v<T, CombinedTextureSampler>) {
                auto texture_sampler = static_cast<CombinedTextureSampler>(*descriptor);
                VkImageView view = static_cast<VulkanTexture*>(texture_sampler.texture)->view();
                VkSampler sampler = static_cast<VulkanSampler*>(texture_sampler.sampler)->handle();
                VkImageLayout layout = write.type == DescriptorType::StorageTexture ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_infos.push_back({ sampler, view, layout });
                image_info_ptr = &image_infos.back();
            }
        }, write.data);

        vk_writes.push_back(convert::DescriptorWrite(handle_, write, image_info_ptr, buffer_info_ptr));
    }

    vkUpdateDescriptorSets(device_, writes.size(), vk_writes.data(), 0, nullptr);
}

} // namespace iinf::rhi::vulkan
