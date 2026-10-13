#pragma once

#include <iinf/rhi/render_context.h>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/descriptor_set_layout.h>
#include <iinf/rhi/frame_context.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/pipeline_layout.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>

#include "command_buffer.h"
#include "descriptor_pool.h"
#include "swapchain.h"
#include "texture.h"

namespace iinf::rhi::vulkan {

class VulkanRenderContext final : public RenderContext {
public:
    static std::unique_ptr<RenderContext> Create(RenderContextCreateInfo info);

    VulkanRenderContext();
    ~VulkanRenderContext() override;

    VulkanRenderContext(const VulkanRenderContext&) = delete;
    VulkanRenderContext& operator=(const VulkanRenderContext&) = delete;
    VulkanRenderContext(VulkanRenderContext&& other) noexcept;

    std::unique_ptr<Sampler> CreateSampler(SamplerCreateInfo info) override;
    std::unique_ptr<Buffer> CreateBuffer(BufferCreateInfo info) override;
    std::unique_ptr<Texture> CreateTexture(TextureCreateInfo info) override;
    std::unique_ptr<Shader> CreateShader(ShaderCreateInfo info) override;
    std::unique_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(DescriptorSetLayoutCreateInfo info) override;
    std::unique_ptr<PipelineLayout> CreatePipelineLayout(PipelineLayoutCreateInfo info) override;
    std::unique_ptr<Pipeline> CreateGraphicsPipeline(GraphicsPipelineCreateInfo info) override;
    std::unique_ptr<Pipeline> CreateComputePipeline(ComputePipelineCreateInfo info) override;
    std::unique_ptr<DescriptorSet> AllocateDescriptorSet(DescriptorSetLayout& layout) override;

    void Upload(Buffer& buffer, void* data, uint32 size, uint32 offset) override;

    FrameContext& BeginFrame(uint32 swapchain_index) override;
    void EndFrame(uint32 swapchain_index) override;
    CommandBuffer& BeginImmediate() override;
    void EndImmediate() override;

    void WaitIdle() override;

    void GenerateMipmaps(VulkanTexture& texture);

    VkInstance instance() const { return instance_; }
    VkDebugUtilsMessengerEXT debug_messenger() const { return debug_messenger_; }
    VkPhysicalDevice physical_device() const { return physical_device_; }
    VkDevice device() const { return device_; }
    VkSurfaceKHR surface() const { return surface_; }
    VkQueue graphics_queue() const { return graphics_queue_; }
    uint32 graphics_queue_family() const { return graphics_queue_family_; }
    VmaAllocator allocator() const { return allocator_; }
    std::vector<VulkanSwapchain>& swapchains() { return swapchains_; }

private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
    VkPhysicalDevice physical_device_;
    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue graphics_queue_;
    uint32 graphics_queue_family_;
    VmaAllocator allocator_;
    std::vector<VulkanSwapchain> swapchains_;

    VulkanBumpDescriptorPool descriptor_pool_;
    VkCommandPool command_pool_;
    VulkanCommandBuffer command_buffer_;
    VkFence immediate_fence_;
};

} // namespace iinf::rhi::vulkan 
