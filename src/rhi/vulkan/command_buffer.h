#pragma once

#include "pipeline.h"
#include <iinf/rhi/command_buffer.h>

#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>
#include <iinf/rhi/types.h>
#include <vulkan/vulkan_core.h>

namespace iinf::rhi::vulkan {

class VulkanCommandBuffer final : public CommandBuffer {
public:
    VulkanCommandBuffer();

    CommandBuffer& BeginRendering(RenderingInfo rendering_info) override;
    void EndRendering() override;

    void BindPipeline(Pipeline& pipeline) override;
    void BindDescriptorSet(uint32 set_index, DescriptorSet& set) override;
    void BindVertexBuffer(uint32 binding, std::span<Buffer*> buffers, std::span<uint32> offsets) override;
    void BindIndexBuffer(Buffer& buffer, IndexType index_type, uint32 offset) override;
    void SetViewport(Viewport viewport) override;
    void SetScissor(Rect2D scissor) override;

    void PushConstants(ShaderStage stages, void* data, uint32 size, uint32 offset) override;

    void DrawIndexed(uint32 index_count, uint32 instance_count, uint32 first_index, int32 vertex_offset, uint32 first_instance) override;
    void DrawUnindexed(uint32 vertex_count, uint32 instance_count, uint32 first_vertex, uint32 first_instance) override;
    void Dispatch(uint32 group_count_x, uint32 group_count_y, uint32 group_count_z) override;

    void Transition(Texture& texture, TextureLayout layout) override;
    void Copy(Buffer& src_buffer, Buffer& dst_buffer, std::span<BufferCopyRegion> regions) override;
    void Copy(Buffer& src_buffer, Texture& dst_texture, std::span<BufferTextureCopyRegion> regions) override;
    void Copy(Texture& src_texture, Texture& dst_texture, std::span<TextureCopyRegion> regions) override;
    void Blit(Texture& src_texture, Texture& dst_texture, std::span<BlitRegion> regions, Filter filter) override;

    VkCommandBuffer handle() const { return handle_; }
    void set_handle(VkCommandBuffer handle) { handle_ = handle; }

private:
    VkCommandBuffer handle_;
    VulkanPipeline* bound_pipeline_;
};

} // namespace iinf::rhi::vulkan

