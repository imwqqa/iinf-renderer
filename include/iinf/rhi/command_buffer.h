#pragma once

#include <array>
#include <optional>
#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

struct Viewport {
    float x = 0.0F;
    float y = 0.0F;
    float width;
    float height;
    float min_depth;
    float max_depth;
};

struct BufferCopyRegion {
    uint32 size;
    uint32 src_offset = 0;
    uint32 dst_offset = 0;
};

struct BufferTextureCopyRegion {
    uint32 buffer_row_length = 0;
    uint32 buffer_image_height = 0;
    uint32 buffer_offset = 0;
    uint32 texture_mip_level = 0;
    uint32 texture_base_array_layer = 0;
    uint32 texture_layer_count = 1;
    Extent3D texture_extent;
    Offset3D texture_offset = { 0, 0, 0 };
};

struct TextureSubresource {
    uint32 mip_level = 0;
    uint32 base_array_layer = 0;
    uint32 layer_count = 1;
};

struct TextureCopyRegion {
    TextureSubresource src_subresource;
    Offset3D src_offset = { 0, 0, 0 };
    TextureSubresource dst_subresource;
    Offset3D dst_offset = { 0, 0, 0 };
    Extent3D extent;
};

struct BlitRegion {
    TextureSubresource src_subresource;
    std::array<Offset3D, 2> src_offsets;
    TextureSubresource dst_subresource;
    std::array<Offset3D, 2> dst_offsets;
};

enum class LoadOp : uint8 {
    Undefined = 0,
    Load,
    Clear,
    DontCare
};

enum class StoreOp : uint8 {
    Undefined = 0,
    Store,
    DontCare
};

struct ClearColorValue {
    float r = 0.0F;
    float g = 0.0F;
    float b = 0.0F;
    float a = 1.0F;
};

struct ClearDepthStencilValue {
    float depth = 0.0F;
    uint32 stencil = 0;
};

struct ColorAttachment {
    Texture* texture;
    LoadOp load_op;
    StoreOp store_op;
    ClearColorValue clear_value;
    uint32 mip_level = 0;
    uint32 array_layer = 0;
};

struct DepthStencilAttachment {
    Texture* texture;
    LoadOp depth_load_op;
    StoreOp depth_store_op;
    LoadOp stencil_load_op;
    StoreOp stencil_store_op;
    ClearDepthStencilValue clear_value;
    uint32 mip_level = 0;
    uint32 array_layer = 0;
};

struct RenderingInfo {
    std::span<ColorAttachment> color_attachments;
    std::optional<DepthStencilAttachment> depth_stencil_attachment;
    Rect2D render_area;
};

class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;

    virtual CommandBuffer& BeginRendering(RenderingInfo rendering_info) = 0;
    virtual void EndRendering() = 0;

    virtual void BindPipeline(Pipeline& pipeline) = 0;
    virtual void BindDescriptorSet(uint32 set_index, DescriptorSet& set) = 0;
    virtual void BindVertexBuffer(uint32 binding, std::span<Buffer*> buffers, std::span<uint32> offsets) = 0;
    virtual void BindIndexBuffer(Buffer& buffer, IndexType index_type, uint32 offset) = 0;
    virtual void SetViewport(Viewport viewport) = 0;
    virtual void SetScissor(Rect2D scissor) = 0;

    template <typename T> 
    void PushConstants(T& data, ShaderStage stages = ShaderStage::All, uint32 offset = 0) {
        static_assert(!std::is_pointer_v<T>);
        static_assert(std::is_trivially_copyable_v<T>);
        PushConstants(stages, static_cast<void*>(&data), sizeof(T), offset);
    }
    virtual void PushConstants(ShaderStage stages, void* data, uint32 size, uint32 offset) = 0;

    virtual void DrawIndexed(uint32 index_count, uint32 instance_count, uint32 first_index, int32 vertex_offset, uint32 first_instance) = 0;
    virtual void DrawUnindexed(uint32 vertex_count, uint32 instance_count, uint32 first_vertex, uint32 first_instance) = 0;
    virtual void Dispatch(uint32 group_count_x, uint32 group_count_y, uint32 group_count_z) = 0;

    virtual void Transition(Texture& texture, TextureLayout layout) = 0;
    virtual void Copy(Buffer& src_buffer, Buffer& dst_buffer, std::span<BufferCopyRegion> regions) = 0;
    virtual void Copy(Buffer& src_buffer, Texture& dst_texture, std::span<BufferTextureCopyRegion> regions) = 0;
    virtual void Copy(Texture& src_texture, Texture& dst_texture, std::span<TextureCopyRegion> regions) = 0;
    virtual void Blit(Texture& src_texture, Texture& dst_texture, std::span<BlitRegion> regions, Filter filter) = 0;
};

} // namespace iinf::rhi

