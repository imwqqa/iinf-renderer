#include "render_context.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <span>
#include <utility>
#include <vector>
#include <memory>

#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <minwindef.h>
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

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
#include <iinf/rhi/descriptor.h>
#include <iinf/rhi/render_context.h>
#include <iinf/rhi/types.h>

#include "buffer.h"
#include "command_buffer.h"
#include "descriptor_pool.h"
#include "descriptor_set.h"
#include "descriptor_set_layout.h"
#include "frame_context.h"
#include "helpers.h"
#include "pipeline.h"
#include "pipeline_layout.h"
#include "sampler.h"
#include "shader.h"
#include "swapchain.h"
#include "texture.h"

namespace iinf::rhi {

std::unique_ptr<RenderContext> RenderContext::Create(RenderContextCreateInfo info) {
    return vulkan::VulkanRenderContext::Create(info);
}

} // namespace iinf::rhi

namespace iinf::rhi::vulkan {

constexpr bool enable_validation = true;
std::unique_ptr<RenderContext> VulkanRenderContext::Create(RenderContextCreateInfo info) {
    VulkanRenderContext render_context;

    vkb::InstanceBuilder instance_builder;
    vkb::Instance vkb_instance = instance_builder
        .set_engine_name("IINF")
        .request_validation_layers(enable_validation)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build()
        .value();
    render_context.instance_ = vkb_instance.instance;
    render_context.debug_messenger_ = vkb_instance.debug_messenger;

#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR surface_info = { .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surface_info.hwnd = static_cast<HWND>(info.window_handle);
    surface_info.hinstance = static_cast<HINSTANCE>(info.display_handle);
    vkCreateWin32SurfaceKHR(render_context.instance_, &surface_info, nullptr, &render_context.surface_);
#endif

    VkPhysicalDeviceVulkan13Features features_13 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features_13.dynamicRendering = VK_TRUE;
    features_13.synchronization2 = VK_TRUE;
    VkPhysicalDeviceVulkan12Features features_12 { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features_12.bufferDeviceAddress = VK_TRUE;
    features_12.descriptorIndexing = VK_TRUE;
    vkb::PhysicalDeviceSelector selector{ vkb_instance };
    vkb::PhysicalDevice vkb_physical_device = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features_13)
        .set_required_features_12(features_12)
        .set_surface(render_context.surface_)
        .select()
        .value();
    render_context.physical_device_ = vkb_physical_device.physical_device;

    vkb::DeviceBuilder device_builder{ vkb_physical_device };
    vkb::Device vkb_device = device_builder.build().value();

    render_context.device_ = vkb_device.device;
    render_context.graphics_queue_ = vkb_device.get_queue(vkb::QueueType::graphics).value();
    render_context.graphics_queue_family_ = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = render_context.physical_device_;
    allocator_info.device = render_context.device_;
    allocator_info.instance = render_context.instance_;
    allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocator_info, &render_context.allocator_);
    
    // for every required swapchain
    vkb::SwapchainBuilder swapchain_builder { render_context.physical_device_, render_context.device_, render_context.surface_ };
    for (SwapchainCreateInfo swapchain_info : info.swapchain_infos) {
        VulkanSwapchain swapchain;
        swapchain.set_device(render_context.device_);

        // create swapchain images and views
        vkb::Swapchain vkb_swapchain = swapchain_builder
            .set_desired_format(VkSurfaceFormatKHR {
                .format = convert::Format(swapchain_info.format),
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // todo: support sdr, hdr, ..
            })
            .set_desired_present_mode(swapchain_info.enable_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR)
            .set_desired_extent(swapchain_info.extent.width, swapchain_info.extent.height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();
        std::vector<VkImage> images = vkb_swapchain.get_images().value();
        std::vector<VkImageView> views = vkb_swapchain.get_image_views().value();
        std::vector<VulkanTexture> textures;
        textures.reserve(vkb_swapchain.image_count);
        for (uint32 i = 0; i < vkb_swapchain.image_count; i++) {
            textures.emplace_back( images[i], views[i], swapchain_info.extent, swapchain_info.format, TextureUsage::TransferDst); 
        }
        swapchain.set_handle(vkb_swapchain.swapchain);
        swapchain.set_extent({ swapchain_info.extent.width, swapchain_info.extent.height });
        swapchain.set_textures(std::move(textures));

        // create frame contexts
        swapchain.set_inflight_frames(vkb_swapchain.image_count); // info.inflight_count
        for (uint32 i = 0; i < vkb_swapchain.image_count; i++) { // todo: support mismatch between in-flight frame count and swapchain image count
                                                                 // creating sync resources per swapchain image currently
            VulkanFrameContext frame_context;
            frame_context.set_device(render_context.device_);

            // create sync resources
            VkSemaphore image_avaliable_semaphore, render_finished_semaphore;
            VkFence render_finished_fence;
            VkFenceCreateInfo fence_info = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr };
            fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkSemaphoreCreateInfo semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr };
            vkCreateSemaphore(render_context.device_, &semaphore_info, nullptr, &image_avaliable_semaphore);
            vkCreateSemaphore(render_context.device_, &semaphore_info, nullptr, &render_finished_semaphore);
            vkCreateFence(render_context.device_, &fence_info, nullptr, &render_finished_fence);
            frame_context.set_image_available_semaphore(image_avaliable_semaphore);
            frame_context.set_render_finished_semaphore(render_finished_semaphore);
            frame_context.set_render_finished_fence(render_finished_fence);

            // create command pool
            VkCommandPool command_pool;
            VkCommandPoolCreateInfo cmd_pool_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .pNext = nullptr };
            cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            cmd_pool_info.queueFamilyIndex = render_context.graphics_queue_family_;
            vkCreateCommandPool(render_context.device_, &cmd_pool_info, nullptr, &command_pool);
            frame_context.set_command_pool(command_pool);

            // allocate command buffer
            VkCommandBuffer command_buffer;
            VkCommandBufferAllocateInfo cmd_buffer_allocate_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .pNext = nullptr };
            cmd_buffer_allocate_info.commandPool = frame_context.command_pool();
            cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd_buffer_allocate_info.commandBufferCount = 1;
            vkAllocateCommandBuffers(render_context.device_, &cmd_buffer_allocate_info, &command_buffer);
            frame_context.command_buffer().set_handle(command_buffer);

            swapchain.frame_contexts().emplace_back(std::move(frame_context));
        }

        render_context.swapchains_.emplace_back(std::move(swapchain));
    }

    // init bump descriptor pool
    std::vector<DescriptorPoolRecipeItem> descriptor_recipe = {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 4 },
    };
    render_context.descriptor_pool_ = VulkanBumpDescriptorPool(render_context.device_, std::move(descriptor_recipe), 10);

    // create immediate fence
    VkFence immediate_fence;
    VkFenceCreateInfo fence_info = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr };
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(render_context.device_, &fence_info, nullptr, &immediate_fence);
    render_context.immediate_fence_ = immediate_fence;

    // create immediate command pool
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo cmd_pool_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .pNext = nullptr };
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_info.queueFamilyIndex = render_context.graphics_queue_family_;
    vkCreateCommandPool(render_context.device_, &cmd_pool_info, nullptr, &command_pool);
    render_context.command_pool_ = command_pool;

    // allocate immediate command buffer
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo cmd_buffer_allocate_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .pNext = nullptr };
    cmd_buffer_allocate_info.commandPool = render_context.command_pool_;
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_allocate_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(render_context.device_, &cmd_buffer_allocate_info, &command_buffer);
    render_context.command_buffer_.set_handle(command_buffer);
    
    return std::make_unique<VulkanRenderContext>(std::move(render_context));
}

VulkanRenderContext::VulkanRenderContext()
    : instance_(VK_NULL_HANDLE), surface_(VK_NULL_HANDLE), debug_messenger_(VK_NULL_HANDLE), 
    physical_device_(VK_NULL_HANDLE), device_(VK_NULL_HANDLE), 
    command_pool_(VK_NULL_HANDLE), graphics_queue_(VK_NULL_HANDLE), graphics_queue_family_(0),
    allocator_(nullptr),
    immediate_fence_(VK_NULL_HANDLE) {
}

VulkanRenderContext::~VulkanRenderContext() {
    if (device_ == VK_NULL_HANDLE) {
        return;
    }
    vkDeviceWaitIdle(device_);
    swapchains_.clear();
    descriptor_pool_ = {};
    vkDestroyCommandPool(device_, command_pool_, nullptr);
    vkDestroyFence(device_, immediate_fence_, nullptr);
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkb::destroy_debug_utils_messenger(instance_, debug_messenger_);
    vkDestroyInstance(instance_, nullptr);
}

VulkanRenderContext::VulkanRenderContext(VulkanRenderContext&& other) noexcept : VulkanRenderContext() {
    std::swap(instance_, other.instance_);
    std::swap(debug_messenger_, other.debug_messenger_);
    std::swap(physical_device_, other.physical_device_);
    std::swap(device_, other.device_);
    std::swap(surface_, other.surface_);
    std::swap(graphics_queue_, other.graphics_queue_);
    std::swap(graphics_queue_family_, other.graphics_queue_family_);
    std::swap(allocator_, other.allocator_);
    std::swap(swapchains_, other.swapchains_);
    std::swap(descriptor_pool_, other.descriptor_pool_);
    std::swap(command_pool_, other.command_pool_);
    std::swap(command_buffer_, other.command_buffer_);
    std::swap(immediate_fence_, other.immediate_fence_);
}

std::unique_ptr<Sampler> VulkanRenderContext::CreateSampler(SamplerCreateInfo info) {
    VkSamplerCreateInfo sampler_info = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
    sampler_info.maxLod = VK_LOD_CLAMP_NONE;
    sampler_info.minLod = 0;
    sampler_info.magFilter = convert::Filter(info.mag_filter);
    sampler_info.minFilter = convert::Filter(info.min_filter);
    sampler_info.mipmapMode = convert::MipmapFilter(info.mip_filter);
    sampler_info.addressModeU = convert::AddressMode(info.address_mode_u);
    sampler_info.addressModeV = convert::AddressMode(info.address_mode_v);
    sampler_info.addressModeW = convert::AddressMode(info.address_mode_w);
    sampler_info.anisotropyEnable = true;
    sampler_info.maxAnisotropy = info.max_anisotropy;
    sampler_info.compareEnable = info.enable_compare;
    sampler_info.compareOp = convert::CompareOp(info.compare_op);
    sampler_info.borderColor = convert::BorderColor(info.border_color);

    VkSampler sampler;
    vkCreateSampler(device_, &sampler_info, nullptr, &sampler);

    return std::make_unique<VulkanSampler>(device_, sampler);
}

std::unique_ptr<Buffer> VulkanRenderContext::CreateBuffer(BufferCreateInfo info) {
    VkBufferCreateInfo buffer_info = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .pNext = nullptr };
    buffer_info.size = info.size;
    buffer_info.usage = convert::BufferUsage(info.buffer_usage);

    auto [usage, flags] = convert::MemoryUsage(info.memory_usage);
    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = usage;
    alloc_create_info.flags = flags;

    VkBuffer buffer;
    VmaAllocation alloc;
    VmaAllocationInfo alloc_info;
    vmaCreateBuffer(allocator_, &buffer_info, &alloc_create_info, &buffer, &alloc, &alloc_info);

    return std::make_unique<VulkanBuffer>(
        device_, allocator_,
        buffer, alloc, alloc_info,
        info.size, info.buffer_usage
    );
}

std::unique_ptr<Texture> VulkanRenderContext::CreateTexture(TextureCreateInfo info) { // todo: support 3d and texture
    VkImageCreateInfo image_info = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .pNext = nullptr };
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent = { info.extent.width, info.extent.height, 1 };
    image_info.format = convert::Format(info.format);
    uint32 mip_levels = info.mip_mode == MipMode::Fullchain ? static_cast<uint32>(std::floor(std::log2(std::max(info.extent.width, info.extent.height)))) + 1 : 1;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = info.array_layers;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = convert::TextureUsage(info.texture_usages);
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    auto [usage, flags] = convert::MemoryUsage(info.memory_usage);
    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = usage;
    allocation_create_info.flags = flags;

    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    vmaCreateImage(allocator_, &image_info, &allocation_create_info, &image, &allocation, &allocation_info);

    VkImageAspectFlags aspect = extract::ImageAspect(info.format);
    VkImageViewCreateInfo view_info = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .pNext = nullptr };
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.image = image;
    view_info.format = image_info.format;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = image_info.mipLevels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = image_info.arrayLayers;
    view_info.subresourceRange.aspectMask = aspect;

    VkImageView view;
    vkCreateImageView(device_, &view_info, nullptr, &view);

    auto texture = std::make_unique<VulkanTexture>(
        device_, allocator_,
        image, view, allocation, allocation_info,
        info.extent, info.format, mip_levels, info.array_layers, info.texture_usages
    );

    GenerateMipmaps(*texture);

    return texture;
}

std::unique_ptr<Shader> VulkanRenderContext::CreateShader(ShaderCreateInfo info) {
    VkShaderModuleCreateInfo shader_module_info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .pNext = nullptr };
    shader_module_info.codeSize = info.bytecode.size() * sizeof(uint32);
    shader_module_info.pCode = info.bytecode.data();

    VkShaderModule shader_module;
    vkCreateShaderModule(device_, &shader_module_info, nullptr, &shader_module);

    return std::make_unique<VulkanShader>(device_, shader_module, info.entry_point);
}

std::unique_ptr<DescriptorSetLayout> VulkanRenderContext::CreateDescriptorSetLayout(DescriptorSetLayoutCreateInfo info) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (Descriptor descriptor : info.descriptors) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = descriptor.binding;
        binding.stageFlags = convert::ShaderStage(descriptor.stages);
        binding.descriptorType = convert::DescriptorType(descriptor.type);
        binding.descriptorCount = 1;
        bindings.push_back(binding);
    }   

    VkDescriptorSetLayoutCreateInfo layout_info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .pNext = nullptr };
    layout_info.pBindings = bindings.data();
    layout_info.bindingCount = bindings.size();
    layout_info.flags = 0;

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device_, &layout_info, nullptr, &layout);

    return std::make_unique<VulkanDescriptorSetLayout>(device_, layout);
}

std::unique_ptr<PipelineLayout> VulkanRenderContext::CreatePipelineLayout(PipelineLayoutCreateInfo info) {
    std::vector<VkDescriptorSetLayout> set_layouts;
    std::vector<VkPushConstantRange> constant_ranges;
    for (DescriptorSetLayout* set_layout : info.descriptor_set_layouts) {
        auto* vk_set_layout = static_cast<VulkanDescriptorSetLayout*>(set_layout);
        set_layouts.push_back(vk_set_layout->handle());
    }
    for (const PushConstantRange& constant_range : info.constant_ranges) {
        VkPushConstantRange vk_constant_range;
        vk_constant_range.offset = constant_range.offset;
        vk_constant_range.size = constant_range.size;
        vk_constant_range.stageFlags = convert::ShaderStage(constant_range.stages);
        constant_ranges.push_back(vk_constant_range);
    }   

    VkPipelineLayoutCreateInfo pipeline_layout_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pNext = nullptr };
    pipeline_layout_info.pSetLayouts = set_layouts.data();
    pipeline_layout_info.setLayoutCount = set_layouts.size();
    pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
    pipeline_layout_info.pushConstantRangeCount = constant_ranges.size();

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr, &pipeline_layout);

    return std::make_unique<VulkanPipelineLayout>(device_, pipeline_layout);
}

std::unique_ptr<Pipeline> VulkanRenderContext::CreateGraphicsPipeline(GraphicsPipelineCreateInfo info) {
    std::vector<VkFormat> color_attachment_formats = convert::Formats(info.color_attachment_formats);
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states = convert::BlendStates(info.color_attachment_blend_states);
    std::vector<VkPipelineShaderStageCreateInfo> shader_infos;
    VkPipelineShaderStageCreateInfo shader_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = nullptr };
    if (info.vertex_shader) { 
        shader_info.module = static_cast<VulkanShader*>(info.vertex_shader)->handle();
        shader_info.pName = info.vertex_shader->entry_point().c_str();
        shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_infos.push_back(shader_info);
    }
    if (info.fragment_shader) {
        shader_info.module = static_cast<VulkanShader*>(info.fragment_shader)->handle();
        shader_info.pName = info.fragment_shader->entry_point().c_str();
        shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_infos.push_back(shader_info);
    }
    std::vector<VkVertexInputAttributeDescription> input_attributes = convert::VertexAttributes(info.vertex_attributes);
    std::vector<VkVertexInputBindingDescription> input_bindings = convert::VertexInputs(info.vertex_inputs);
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = convert::InputAssemblyState(info.input_assembly_state);
    VkPipelineMultisampleStateCreateInfo multisample_state = convert::MultisampleState(); // todo
    VkPipelineRasterizationStateCreateInfo rasterization_state = convert::RasterizationState(info.rasterization_state);
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = convert::DepthStencilState(info.depth_stencil_state);

    VkPipelineRenderingCreateInfo render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, .pNext = nullptr };
    render_info.pColorAttachmentFormats = color_attachment_formats.data();
    render_info.colorAttachmentCount = info.color_attachment_formats.size();
    render_info.depthAttachmentFormat = convert::Format(info.depth_attachment_format);
    // render_info.stencilAttachmentFormat = convert::Format(info.stencil_attachment_format); not support yet

    VkPipelineViewportStateCreateInfo viewport_state_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .pNext = nullptr };
    viewport_state_info.viewportCount = 1;
    viewport_state_info.scissorCount = 1;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .pNext = nullptr };
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_info.pAttachments = color_blend_attachment_states.data();
    color_blend_state_info.attachmentCount = color_blend_attachment_states.size();

    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, .pNext = nullptr };
    vertex_input_state_info.pVertexAttributeDescriptions = input_attributes.data();
    vertex_input_state_info.vertexAttributeDescriptionCount = input_attributes.size();
    vertex_input_state_info.pVertexBindingDescriptions = input_bindings.data();
    vertex_input_state_info.vertexBindingDescriptionCount = input_bindings.size();

    std::array<VkDynamicState, 2> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.pDynamicStates = dynamic_states.data();
    dynamic_state_info.dynamicStateCount = 2;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = &render_info;
    pipeline_info.pStages = shader_infos.data();
    pipeline_info.stageCount = shader_infos.size();
    pipeline_info.pVertexInputState = &vertex_input_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly_state;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state;
    pipeline_info.pMultisampleState = &multisample_state;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_state;
    pipeline_info.layout = static_cast<VulkanPipelineLayout*>(info.pipeline_layout)->handle();
    pipeline_info.pDynamicState = &dynamic_state_info;

    VkPipeline pipeline;
    vkCreateGraphicsPipelines(device_, nullptr, 1, &pipeline_info, nullptr, &pipeline);

    return std::make_unique<VulkanPipeline>(device_, pipeline, PipelineType::Graphics, *info.pipeline_layout);
}

std::unique_ptr<Pipeline> VulkanRenderContext::CreateComputePipeline(ComputePipelineCreateInfo info) {
    // todo: support compute pipeline
}

std::unique_ptr<DescriptorSet> VulkanRenderContext::AllocateDescriptorSet(DescriptorSetLayout& layout) {
    VkDescriptorSet set;

    set = descriptor_pool_.Allocate(static_cast<VulkanDescriptorSetLayout&>(layout));

    return std::make_unique<VulkanDescriptorSet>(device_, set);
}

void VulkanRenderContext::Upload(Buffer& dst_buffer, void* data, uint32 size, uint32 offset) {
    BufferCreateInfo buffer_info = {
        .size = size,
        .buffer_usage = BufferUsage::TransferSrc,
        .memory_usage = MemoryUsage::Staging 
    };
    std::unique_ptr<Buffer> staging_buffer = CreateBuffer(buffer_info);

    void* dst = staging_buffer->Map();
    memcpy(dst, data, size);
    staging_buffer->Unmap();

    CommandBuffer& command_buffer = BeginImmediate();

    std::vector<BufferCopyRegion> regions;
    regions.push_back({ .size = size, .dst_offset = offset });
    command_buffer.Copy(*staging_buffer, dst_buffer, regions);

    EndImmediate();
}

FrameContext& VulkanRenderContext::BeginFrame(uint32 swapchain_index) {
    VulkanFrameContext& frame_context = swapchains_[swapchain_index].current_frame_context();
    VulkanSwapchain& swapchain = swapchains_[swapchain_index];
    VulkanCommandBuffer& command_buffer = frame_context.command_buffer();

    auto render_timeout_ns = std::chrono::nanoseconds(std::chrono::seconds(1)).count();
    auto acquire_timeout_ns = std::chrono::nanoseconds(std::chrono::seconds(1)).count();
    VkFence render_finished_fence = frame_context.render_finished_fence();
    vkWaitForFences(device_, 1, &render_finished_fence, true, render_timeout_ns);

    // todo: support temp descriptor set allocation
    // frame_context.descriptor_pool.Clear()

    {
        uint32 index;
        VkResult result = vkAcquireNextImageKHR(
            device_, swapchain.handle(), acquire_timeout_ns,
            frame_context.image_available_semaphore(), nullptr, &index);
        swapchain.set_swapchain_image_index(index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // is_resize_requested = true
            // todo: support swapchain resizing
        }
    }

    frame_context.set_present_texture(swapchains_[swapchain_index].textures()[swapchain.swapchain_image_index()]);

    vkResetCommandBuffer(command_buffer.handle(), 0);

    VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr };
    begin_info.pInheritanceInfo = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer.handle(), &begin_info);

    return frame_context;
}

void VulkanRenderContext::EndFrame(uint32 swapchain_index) {
    VulkanFrameContext& frame_context = swapchains_[swapchain_index].current_frame_context();
    VulkanSwapchain& swapchain = swapchains_[swapchain_index];
    VulkanCommandBuffer& command_buffer = frame_context.command_buffer();
    VkSemaphore render_finished_semaphore = swapchains_[swapchain_index].frame_contexts()[swapchain.swapchain_image_index()].render_finished_semaphore();

    VkFence render_finished_fence = frame_context.render_finished_fence();
    vkResetFences(device_, 1, &render_finished_fence);

    // command_buffer.Transition(frame_context.present_texture(), TextureLayout::Present);

    vkEndCommandBuffer(command_buffer.handle());

    VkCommandBufferSubmitInfo command_buffer_submit_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .pNext = nullptr };
    command_buffer_submit_info.commandBuffer = command_buffer.handle();
    command_buffer_submit_info.deviceMask = 0;

    VkSemaphoreSubmitInfo wait_semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .pNext = nullptr }; 
    wait_semaphore_info.semaphore = frame_context.image_available_semaphore();
    wait_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
    wait_semaphore_info.deviceIndex = 0;
    wait_semaphore_info.value = 1;

    VkSemaphoreSubmitInfo signal_semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .pNext = nullptr }; 
    signal_semaphore_info.semaphore = render_finished_semaphore; // frame_context.render_finished_semaphore();
    signal_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    signal_semaphore_info.deviceIndex = 0;
    signal_semaphore_info.value = 1;

    VkSubmitInfo2 submit_info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .pNext = nullptr };
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_semaphore_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_buffer_submit_info;

    vkQueueSubmit2(graphics_queue_, 1, &submit_info, frame_context.render_finished_fence());

    VkSwapchainKHR swapchain_handle = swapchain.handle();
    // VkSemaphore render_finished_semaphore = frame_context.render_finished_semaphore();

    uint32 swapchain_image_index = swapchain.swapchain_image_index();
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.pSwapchains = &swapchain_handle;
    present_info.swapchainCount = 1;
    present_info.pWaitSemaphores = &render_finished_semaphore;
    present_info.waitSemaphoreCount = 1;
    present_info.pImageIndices = &swapchain_image_index;

    {
        VkResult result = vkQueuePresentKHR(graphics_queue_, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // is_resize_requested_ = true
            // todo: support swapchain resizing
        }
    }

    swapchain.set_inflight_index((swapchain.inflight_index() + 1) % swapchain.inflight_frames());
}

CommandBuffer& VulkanRenderContext::BeginImmediate() {
    vkResetFences(device_, 1, &immediate_fence_);
    vkResetCommandBuffer(command_buffer_.handle(), 0);

    VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr };
    begin_info.pInheritanceInfo = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer_.handle(), &begin_info);

    return command_buffer_;
}

void VulkanRenderContext::EndImmediate() {
    vkEndCommandBuffer(command_buffer_.handle());

    VkCommandBufferSubmitInfo command_buffer_submit_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .pNext = nullptr };
    command_buffer_submit_info.commandBuffer = command_buffer_.handle();
    command_buffer_submit_info.deviceMask = 0;

    VkSubmitInfo2 submit_info = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .pNext = nullptr };
    submit_info.waitSemaphoreInfoCount = 0;
    submit_info.pWaitSemaphoreInfos = nullptr;
    submit_info.signalSemaphoreInfoCount = 0;
    submit_info.pSignalSemaphoreInfos = nullptr;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_buffer_submit_info;

    vkQueueSubmit2(graphics_queue_, 1, &submit_info, immediate_fence_);
    auto immediate_timeout_ns = std::chrono::nanoseconds(std::chrono::seconds(100)).count();
    vkWaitForFences(device_, 1, &immediate_fence_, true, immediate_timeout_ns);
}

void VulkanRenderContext::WaitIdle() {
    vkDeviceWaitIdle(device_);
}

void VulkanRenderContext::GenerateMipmaps(VulkanTexture& texture) {
    VkImage image = texture.handle();
    uint32 mip_levels = texture.mip_levels();
    
    if (mip_levels <= 1) {
        return;
    }
    
    VkImageAspectFlags aspect_mask = extract::ImageAspect(texture.format());
    Extent2D extent = texture.extent();
    uint32 array_layers = texture.array_layers();
    
    uint32 src_width = extent.width;
    uint32 src_height = extent.height;
    
    auto& cmd = static_cast<VulkanCommandBuffer&>(BeginImmediate());
    for (uint32 i = 0; i < mip_levels - 1; i++) {
        uint32 dst_width = src_width > 1 ? src_width / 2 : 1;
        uint32 dst_height = src_height > 1 ? src_height / 2 : 1;
        {
            VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspect_mask;
            barrier.subresourceRange.baseMipLevel = i;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = array_layers;
            
            VkDependencyInfo dep_info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
            dep_info.imageMemoryBarrierCount = 1;
            dep_info.pImageMemoryBarriers = &barrier;
            
            vkCmdPipelineBarrier2(cmd.handle(), &dep_info);
        }
        {
            VkImageBlit blit_region = {};
            blit_region.srcSubresource.aspectMask = aspect_mask;
            blit_region.srcSubresource.mipLevel = i;
            blit_region.srcSubresource.baseArrayLayer = 0;
            blit_region.srcSubresource.layerCount = array_layers;
            blit_region.srcOffsets[0] = { 0, 0, 0 };
            blit_region.srcOffsets[1] = { static_cast<int32>(src_width), static_cast<int32>(src_height), 1 };
            blit_region.dstSubresource.aspectMask = aspect_mask;
            blit_region.dstSubresource.mipLevel = i + 1;
            blit_region.dstSubresource.baseArrayLayer = 0;
            blit_region.dstSubresource.layerCount = array_layers;
            blit_region.dstOffsets[0] = { 0, 0, 0 };
            blit_region.dstOffsets[1] = { static_cast<int32>(dst_width), static_cast<int32>(dst_height), 1 };
            
            vkCmdBlitImage(cmd.handle(), 
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit_region, VK_FILTER_LINEAR);
        }
        {
            VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
            barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspect_mask;
            barrier.subresourceRange.baseMipLevel = i;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = array_layers;
            
            VkDependencyInfo dep_info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
            dep_info.imageMemoryBarrierCount = 1;
            dep_info.pImageMemoryBarriers = &barrier;
            
            vkCmdPipelineBarrier2(cmd.handle(), &dep_info);
        }
        
        src_width = dst_width;
        src_height = dst_height;
    }

    VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect_mask;
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = array_layers;
        
    VkDependencyInfo dep_info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &barrier;
    
    vkCmdPipelineBarrier2(cmd.handle(), &dep_info);
    
    texture.set_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

} // namespace iinf::rhi::vulkan
