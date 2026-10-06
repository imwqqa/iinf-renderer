#pragma once

#include <memory>
#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/frame_context.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/pipeline_layout.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

struct SwapchainCreateInfo {
    void* native_window_handle;
    Extent2D extent;
    Format format = Format::BGRA8UNormGamma;
    uint32 image_count = 3;
    bool enable_vsync = true;
};

struct RenderContextCreateInfo {
    std::span<SwapchainCreateInfo> swapchain_infos;
    uint32 inflight_count = 3;
};

class RenderContext {
public:
    static std::unique_ptr<RenderContext> Create(RenderContextCreateInfo info);
    virtual ~RenderContext() = default;
    
    virtual std::unique_ptr<Buffer> CreateBuffer(BufferCreateInfo info) = 0;
    virtual std::unique_ptr<Texture> CreateTexture(TextureCreateInfo info) = 0;
    virtual std::unique_ptr<Sampler> CreateSampler(SamplerCreateInfo info) = 0;
    virtual std::unique_ptr<Shader> CreateShader(ShaderCreateInfo info) = 0;
    virtual std::unique_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(DescriptorSetLayoutCreateInfo info) = 0;
    virtual std::unique_ptr<PipelineLayout> CreatePiplineLayout(PipelineLayoutCreateInfo info) = 0;
    virtual std::unique_ptr<Pipeline> CreateGraphicsPipeline(GraphicsPipelineCreateInfo info) = 0;
    virtual std::unique_ptr<Pipeline> CreateComputePipline(ComputePipelineCreateInfo info) = 0;

    virtual std::unique_ptr<DescriptorSet> AllocateDescriptorSet(const DescriptorSetLayout* layout) = 0;

    virtual void Upload(Buffer* buffer, void* data, uint32 size, uint32 offset) = 0;
    template<typename T>
    void Upload(Buffer* buffer, T* data, uint32 offset = 0) {
        static_assert(!std::is_pointer_v<T>);
        static_assert(std::is_trivially_copyable_v<T>);
        Upload(buffer, data, sizeof(T), offset);
    }

    virtual FrameContext& BeginFrame(uint8 swapchain_index) = 0;
    virtual void EndFrame() = 0;
    virtual CommandBuffer& BeginImmediate() = 0;
    virtual void EndImmediate() = 0;

    virtual void WaitIdle() = 0;
};

} // namespace iinf::rhi
