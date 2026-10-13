#include "command_buffer.h"

#include <span>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/texture.h>
#include <iinf/rhi/types.h>

#include "buffer.h"
#include "descriptor_set.h"
#include "pipeline.h"
#include "texture.h"
#include "helpers.h"

namespace iinf::rhi::vulkan {

VulkanCommandBuffer::VulkanCommandBuffer() {
    handle_ = VK_NULL_HANDLE;
    bound_pipeline_ = nullptr;
}

CommandBuffer& VulkanCommandBuffer::BeginRendering(RenderingInfo info) {
    std::vector<VkRenderingAttachmentInfo> color_attachment_infos;
    VkRenderingAttachmentInfo depth_stencil_attachment_info;

    for (ColorAttachment color_attachment : info.color_attachments) {
        auto* texture = static_cast<VulkanTexture*>(color_attachment.texture);

        VkRenderingAttachmentInfo attachment_info = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, .pNext = nullptr };
        attachment_info.imageView = texture->view(color_attachment.mip_level, color_attachment.array_layer);
        attachment_info.imageLayout = convert::TextureLayout(texture->layout());
        attachment_info.loadOp = convert::LoadOp(color_attachment.load_op);
        attachment_info.storeOp = convert::StoreOp(color_attachment.store_op);
        attachment_info.clearValue = { .color = convert::ClearColorValue(color_attachment.clear_value) };
        color_attachment_infos.push_back(attachment_info);
    }
    if (info.depth_stencil_attachment) {
        DepthStencilAttachment depth_stencil_attachment = *info.depth_stencil_attachment;
        auto* texture = static_cast<VulkanTexture*>(depth_stencil_attachment.texture);

        VkRenderingAttachmentInfo attachment_info = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, .pNext = nullptr };
        attachment_info.imageView = texture->view(depth_stencil_attachment.mip_level, depth_stencil_attachment.array_layer);
        attachment_info.imageLayout = convert::TextureLayout(texture->layout());
        attachment_info.loadOp = convert::LoadOp(depth_stencil_attachment.depth_load_op);
        attachment_info.storeOp = convert::StoreOp(depth_stencil_attachment.depth_store_op);
        attachment_info.clearValue = { .depthStencil = convert::ClearDepthStencilValue(depth_stencil_attachment.clear_value) };
        depth_stencil_attachment_info = attachment_info;
    }

    VkRenderingInfo rendering_info = { .sType = VK_STRUCTURE_TYPE_RENDERING_INFO, .pNext = nullptr };
    rendering_info.renderArea = convert::Rect2D(info.render_area);
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = color_attachment_infos.size();
    rendering_info.pColorAttachments = info.color_attachments.size() ? color_attachment_infos.data() : nullptr;
    rendering_info.pDepthAttachment = info.depth_stencil_attachment ? &depth_stencil_attachment_info : nullptr;
    rendering_info.pStencilAttachment = nullptr;

    vkCmdBeginRendering(handle_, &rendering_info);

    return *this;
}

void VulkanCommandBuffer::EndRendering() {
    vkCmdEndRendering(handle_);
}

void VulkanCommandBuffer::BindPipeline(Pipeline& pipeline) {
    bound_pipeline_ = static_cast<VulkanPipeline*>(&pipeline);
    VkPipelineBindPoint bind_point = bound_pipeline_->type() == PipelineType::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    vkCmdBindPipeline(handle_, bind_point, bound_pipeline_->handle());
}

void VulkanCommandBuffer::BindDescriptorSet(uint32 set_index, DescriptorSet& set) {
    VkPipelineBindPoint bind_point = bound_pipeline_->type() == PipelineType::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    VkDescriptorSet vk_set = static_cast<VulkanDescriptorSet&>(set).handle();

    vkCmdBindDescriptorSets(handle_, bind_point, bound_pipeline_->layout_handle(), set_index, 1, &vk_set, 0, nullptr);
}

void VulkanCommandBuffer::BindVertexBuffer(uint32 binding, std::span<Buffer*> buffers, std::span<uint32> offsets) {
    std::vector<VkBuffer> vk_buffers;
    for (Buffer* buffer : buffers) {
        vk_buffers.push_back(static_cast<VulkanBuffer*>(buffer)->handle());
    }
    std::vector<uint64> vk_offsets = std::vector<uint64>(offsets.begin(), offsets.end());

    vkCmdBindVertexBuffers(handle_, binding, vk_buffers.size(), vk_buffers.data(), vk_offsets.data());
}

void VulkanCommandBuffer::BindIndexBuffer(Buffer& buffer, IndexType index_type, uint32 offset) {
    vkCmdBindIndexBuffer(handle_, static_cast<VulkanBuffer*>(&buffer)->handle(), offset, convert::IndexType(index_type));
}

void VulkanCommandBuffer::SetViewport(Viewport viewport) { // todo: support multiple viewports
    VkViewport vk_viewport = convert::Viewport(viewport);

    vkCmdSetViewport(handle_, 0, 1, &vk_viewport);
}

void VulkanCommandBuffer::SetScissor(Rect2D scissor) { // todo: support multiple scissors
    VkRect2D vk_scissor = convert::Rect2D(scissor);

    vkCmdSetScissor(handle_, 0, 1, &vk_scissor);
}

void VulkanCommandBuffer::PushConstants(ShaderStage stages, void* data, uint32 size, uint32 offset) {
    vkCmdPushConstants(handle_, bound_pipeline_->layout_handle(), convert::ShaderStage(stages), offset, size, data);
}

void VulkanCommandBuffer::DrawIndexed(uint32 index_count, uint32 instance_count, uint32 first_index, int32 vertex_offset, uint32 first_instance) {
    vkCmdDrawIndexed(handle_, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void VulkanCommandBuffer::DrawUnindexed(uint32 vertex_count, uint32 instance_count, uint32 first_vertex, uint32 first_instance) {
    vkCmdDraw(handle_, vertex_count, instance_count, first_vertex, first_instance);
}

void VulkanCommandBuffer::Dispatch(uint32 group_count_x, uint32 group_count_y, uint32 group_count_z) {
    vkCmdDispatch(handle_, group_count_x, group_count_y, group_count_z);
}

void VulkanCommandBuffer::Transition(Texture& texture, TextureLayout layout) {
    VkImage image = static_cast<VulkanTexture*>(&texture)->handle();
    auto [src_stages, src_accesses] = extract::BarrierStageAccess2(texture.layout());
    auto [dst_stages, dst_accesses] = extract::BarrierStageAccess2(layout);
    VkImageAspectFlags aspect_mask = extract::ImageAspect(texture.format());

    VkImageSubresourceRange range = {};
    range.aspectMask = aspect_mask;
    range.baseMipLevel = 0;
    range.levelCount = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer = 0;
    range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
    barrier.srcStageMask = src_stages;
    barrier.srcAccessMask = src_accesses;
    barrier.dstStageMask = dst_stages;
    barrier.dstAccessMask = dst_accesses;
    barrier.oldLayout = convert::TextureLayout(texture.layout());
    barrier.newLayout = convert::TextureLayout(layout);
    barrier.subresourceRange = range;
    barrier.image = image;

    VkDependencyInfo dependency_info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
    dependency_info.imageMemoryBarrierCount = 1;
    dependency_info.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(handle_, &dependency_info);
    static_cast<VulkanTexture*>(&texture)->set_layout(convert::TextureLayout(layout));
}

void VulkanCommandBuffer::Copy(Buffer& src_buffer, Buffer& dst_buffer, std::span<BufferCopyRegion> regions) {
    VkBuffer vk_src_buffer = static_cast<VulkanBuffer&>(src_buffer).handle();
    VkBuffer vk_dst_buffer = static_cast<VulkanBuffer&>(dst_buffer).handle();
    std::vector<VkBufferCopy> vk_regions = convert::BufferCopyRegions(regions);

    vkCmdCopyBuffer(handle_, vk_src_buffer, vk_dst_buffer, static_cast<uint32>(vk_regions.size()), vk_regions.data());
}

void VulkanCommandBuffer::Copy(Buffer& src_buffer, Texture& dst_texture, std::span<BufferTextureCopyRegion> regions) {
    VkBuffer vk_src_buffer = static_cast<VulkanBuffer&>(src_buffer).handle();
    VkImage vk_dst_image = static_cast<VulkanTexture&>(dst_texture).handle();
    std::vector<VkBufferImageCopy> vk_regions = convert::BufferTextureCopyRegions(regions, extract::ImageAspect(dst_texture.format()));

    vkCmdCopyBufferToImage(handle_, vk_src_buffer, vk_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32>(vk_regions.size()), vk_regions.data());
}

void VulkanCommandBuffer::Copy(Texture& src_texture, Texture& dst_texture, std::span<TextureCopyRegion> regions) {
    VkImage vk_src_image = static_cast<VulkanTexture&>(src_texture).handle();
    VkImage vk_dst_image = static_cast<VulkanTexture&>(dst_texture).handle();
    VkImageAspectFlags src_aspect = extract::ImageAspect(src_texture.format());
    VkImageAspectFlags dst_aspect = extract::ImageAspect(dst_texture.format());

    std::vector<VkImageCopy> vk_regions = convert::TextureCopyRegions(regions, src_aspect, dst_aspect);

    vkCmdCopyImage(handle_, vk_src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32>(vk_regions.size()), vk_regions.data());
}

void VulkanCommandBuffer::Blit(Texture& src_texture, Texture& dst_texture, std::span<BlitRegion> regions, Filter filter) {
    VkImage vk_src_image = static_cast<VulkanTexture&>(src_texture).handle();
    VkImage vk_dst_image = static_cast<VulkanTexture&>(dst_texture).handle();
    VkImageAspectFlags src_aspect = extract::ImageAspect(src_texture.format());
    VkImageAspectFlags dst_aspect = extract::ImageAspect(dst_texture.format());

    std::vector<VkImageBlit> vk_regions = convert::BlitRegions(regions, src_aspect, dst_aspect);

    vkCmdBlitImage(handle_, vk_src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32>(vk_regions.size()), vk_regions.data(), convert::Filter(filter));
}

} // namespace iinf::rhi::vulkan
