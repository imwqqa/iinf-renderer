#pragma once

#include <cstdlib>
#include <span>
#include <tuple>
#include <vector>

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/descriptor_set.h>
#include <iinf/rhi/frame_context.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/pipeline_layout.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>
#include <iinf/rhi/descriptor.h>
#include <iinf/rhi/render_context.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi::vulkan {

namespace convert {

constexpr VkFilter Filter(Filter filter) {
    switch (filter) {
        case Filter::Nearest: 
            return VkFilter::VK_FILTER_NEAREST; break;
        case Filter::Linear: 
            return VkFilter::VK_FILTER_LINEAR; break;
        case Filter::Undefined:
        default:
            return VkFilter::VK_FILTER_NEAREST;
    }
}

constexpr VkSamplerMipmapMode MipmapFilter(enum MipmapFilter filter) {
    switch (filter) {
        case MipmapFilter::Nearest: 
            return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST; break;
        case MipmapFilter::Linear: 
            return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR; break;
        case MipmapFilter::Undefined:
        default:
            return VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST; break;
    }
}

constexpr VkBufferUsageFlags BufferUsage(BufferUsage usages) {
    VkBufferUsageFlags vk_usages = 0;
    if ((usages & BufferUsage::TransferSrc) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ((usages & BufferUsage::TransferDst) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if ((usages & BufferUsage::Storage) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if ((usages & BufferUsage::Uniform) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if ((usages & BufferUsage::Vertex) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if ((usages & BufferUsage::Index) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if ((usages & BufferUsage::Indirect) != BufferUsage::Undefined) {
        vk_usages |= VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    return vk_usages;
}

using AllocationRequireMents = std::tuple<VmaMemoryUsage, VmaAllocationCreateFlags>;
constexpr AllocationRequireMents MemoryUsage(MemoryUsage usage) {
    switch (usage) {
        case MemoryUsage::Static: 
            return { VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO, 0 };
            break;
        case MemoryUsage::Dynamic: 
            return {
                VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT 
            };
            break;
        case MemoryUsage::Staging: 
            return {
                VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO,
                VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            };
            break;
        case MemoryUsage::Readback:
            return { VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT };
            break;
        case MemoryUsage::Undefined:
        default: 
            // todo: log fatal
            std::abort();
    }
}

constexpr VkImageUsageFlags TextureUsage(TextureUsage usage) {
    VkImageUsageFlags vk_usage = 0;
    if ((usage & TextureUsage::Sampled) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ((usage & TextureUsage::Storage) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ((usage & TextureUsage::TransferSrc) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ((usage & TextureUsage::TransferDst) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if ((usage & TextureUsage::ColorAttachment) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ((usage & TextureUsage::DepthStencilAttachment) != TextureUsage::Undefined) {
        vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return vk_usage;
}

constexpr VkFormat Format(Format format) {
    switch (format) {            
        case Format::R8UNorm:
            return VK_FORMAT_R8_UNORM;
        case Format::R8UNormGamma:
            return VK_FORMAT_R8_SRGB;
        case Format::R8UInt:
            return VK_FORMAT_R8_UINT;
        case Format::RG8UNorm:
            return VK_FORMAT_R8G8_UNORM;
        case Format::RG8UNormGamma:
            return VK_FORMAT_R8G8_SRGB;
        case Format::RG8UInt:
            return VK_FORMAT_R8G8_UINT;
        case Format::RGBA8UNorm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8UNormGamma:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::RGBA8UInt:
            return VK_FORMAT_R8G8B8A8_UINT;
        case Format::BGRA8UNorm:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8UNormGamma:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::BGRA8UInt:
            return VK_FORMAT_B8G8R8A8_UINT;
            
        case Format::R16Float:
            return VK_FORMAT_R16_SFLOAT;
        case Format::R16UInt:
            return VK_FORMAT_R16_UINT;
        case Format::RG16Float:
            return VK_FORMAT_R16G16_SFLOAT;
        case Format::RG16Int:
            return VK_FORMAT_R16G16_SINT;
        case Format::RGBA16Float:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::RGBA16UInt:
            return VK_FORMAT_R16G16B16A16_UINT;
            
        case Format::R32Float:
            return VK_FORMAT_R32_SFLOAT;
        case Format::R32UInt:
            return VK_FORMAT_R32_UINT;
        case Format::RG32Float:
            return VK_FORMAT_R32G32_SFLOAT;
        case Format::RG32UInt:
            return VK_FORMAT_R32G32_UINT;
        case Format::RGBA32Float:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::RGBA32UInt:
            return VK_FORMAT_R32G32B32A32_UINT;
            
        case Format::RG11B10Float:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case Format::RGB10A2UNorm:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            
        case Format::D16UNorm:
            return VK_FORMAT_D16_UNORM;
        case Format::D32Float:
            return VK_FORMAT_D32_SFLOAT;
        case Format::D24UNormS8UInt:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32FloatS8UInt:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;

        case Format::Undefined:
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

inline std::vector<VkFormat> Formats(std::span<const enum Format> formats) {
    std::vector<VkFormat> vk_formats;
    for (enum Format format : formats) {
        vk_formats.push_back(Format(format));
    }
    return vk_formats;
}

constexpr VkDescriptorType DescriptorType(DescriptorType type) {
    switch (type) {
        case DescriptorType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::StorageTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::Undefined:
        default:
            // todo: log fatal
            std::abort();
    }
}

constexpr VkShaderStageFlags ShaderStage(ShaderStage stages) {
    VkShaderStageFlags vk_stages = 0;
    if ((stages & ShaderStage::Vertex) != ShaderStage::Undefined) {
        vk_stages |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if ((stages & ShaderStage::Fragment) != ShaderStage::Undefined) {
        vk_stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if ((stages & ShaderStage::Compute) != ShaderStage::Undefined) {
        vk_stages |= VK_SHADER_STAGE_COMPUTE_BIT;
    }
    return vk_stages;
}

constexpr VkBlendFactor BlendFactor(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::Undefined:
        default:
            return VK_BLEND_FACTOR_ZERO;
    }
}

constexpr VkBlendOp BlendOp(BlendOp op) {
    switch (op) {
        case BlendOp::Add:
            return VK_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min:
            return VK_BLEND_OP_MIN;
        case BlendOp::Max:
            return VK_BLEND_OP_MAX;
        case BlendOp::Undefined:
        default:
            return VK_BLEND_OP_ADD;
    }
}

constexpr VkPipelineColorBlendAttachmentState BlendState(const BlendState& state) {
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    bool blend_enable = 
        state.src_color != BlendFactor::Undefined || state.dst_color != BlendFactor::Undefined ||
        state.src_alpha != BlendFactor::Undefined || state.dst_alpha != BlendFactor::Undefined;
    color_blend_attachment_state.blendEnable = blend_enable ? VK_TRUE : VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = BlendFactor(state.src_color);
    color_blend_attachment_state.dstColorBlendFactor = BlendFactor(state.dst_color);
    color_blend_attachment_state.colorBlendOp = BlendOp(state.color_op);
    color_blend_attachment_state.srcAlphaBlendFactor = BlendFactor(state.src_alpha);
    color_blend_attachment_state.dstAlphaBlendFactor = BlendFactor(state.dst_alpha);
    color_blend_attachment_state.alphaBlendOp = BlendOp(state.alpha_op);
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    return color_blend_attachment_state;
}

inline std::vector<VkPipelineColorBlendAttachmentState> BlendStates(std::span<const struct BlendState> blends) {
    std::vector<VkPipelineColorBlendAttachmentState> vk_blends;
    for (struct BlendState blend : blends) {
        vk_blends.push_back(BlendState(blend));
    }
    return vk_blends;
}

constexpr VkVertexInputRate InputRate(InputRate rate) {
    switch (rate) {
        case InputRate::PerVertex:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case InputRate::PerInstance:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
        case InputRate::Undefined:
        default:
            return VK_VERTEX_INPUT_RATE_VERTEX;
    }
}

inline VkVertexInputAttributeDescription VertexAttribute(const VertexAttribute& attribute) {
    VkVertexInputAttributeDescription vk_attribute;
    vk_attribute.binding = attribute.binding;
    vk_attribute.location = attribute.location;
    vk_attribute.format = Format(attribute.format);
    vk_attribute.offset = attribute.offset;
    return vk_attribute;
}

inline VkVertexInputBindingDescription VertexInput(const VertexInput& input) {
    VkVertexInputBindingDescription vk_binding;
    vk_binding.binding = input.binding;
    vk_binding.inputRate = InputRate(input.input_rate);
    vk_binding.stride = input.stride;
    return vk_binding;
}

inline std::vector<VkVertexInputAttributeDescription> VertexAttributes(std::span<const struct VertexAttribute> attributes) {
    std::vector<VkVertexInputAttributeDescription> vk_attributes;
    vk_attributes.reserve(attributes.size());
    for (const auto& attribute : attributes) {
        vk_attributes.push_back(VertexAttribute(attribute));
    }
    return vk_attributes;
}

inline std::vector<VkVertexInputBindingDescription> VertexInputs(std::span<const struct VertexInput> inputs) {
    std::vector<VkVertexInputBindingDescription> vk_bindings;
    vk_bindings.reserve(inputs.size());
    for (const auto& input : inputs) {
        vk_bindings.push_back(VertexInput(input));
    }
    return vk_bindings;
}

constexpr VkPrimitiveTopology PrimitiveTopology(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::Undefined:
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

constexpr VkPipelineInputAssemblyStateCreateInfo InputAssemblyState(InputAssemblyState state, bool primitive_restart_enable = false) {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .pNext = nullptr };
    input_assembly_state.topology = PrimitiveTopology(state.primitive_topology);
    input_assembly_state.primitiveRestartEnable = primitive_restart_enable ? VK_TRUE : VK_FALSE;
    return input_assembly_state;
}

constexpr VkCullModeFlags CullMode(CullMode cull_mode) {
    switch (cull_mode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        case CullMode::Undefined:
        default:
            return VK_CULL_MODE_BACK_BIT;
    }
}

constexpr VkFrontFace FrontFace(FrontFace front_face) {
    switch (front_face) {
        case FrontFace::ClockWise:
            return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace::CounterClockWise:
        case FrontFace::Undefined:
        default:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

constexpr VkPipelineRasterizationStateCreateInfo RasterizationState(const RasterizationState& state) {
    VkPipelineRasterizationStateCreateInfo rasterization_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .pNext = nullptr };
    rasterization_state.depthClampEnable = VK_FALSE;
    rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode = CullMode(state.cull_mode);
    rasterization_state.frontFace = FrontFace(state.front_face);
    rasterization_state.depthBiasEnable = VK_FALSE;
    rasterization_state.depthBiasConstantFactor = 0.0F;
    rasterization_state.depthBiasClamp = 0.0F;
    rasterization_state.depthBiasSlopeFactor = 0.0F;
    rasterization_state.lineWidth = state.line_width;
    return rasterization_state;
}

// todo: abstract 1x 4x 8x 16x 32x multisample state
constexpr VkPipelineMultisampleStateCreateInfo MultisampleState() {
    VkPipelineMultisampleStateCreateInfo multisample_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .pNext = nullptr };
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state.sampleShadingEnable = VK_FALSE;
    multisample_state.minSampleShading = 1.0F;
    multisample_state.pSampleMask = nullptr;
    multisample_state.alphaToCoverageEnable = VK_FALSE;
    multisample_state.alphaToOneEnable = VK_FALSE;
    return multisample_state;
}

constexpr VkCompareOp CompareOp(CompareOp compare_op) {
    switch (compare_op) {
        case CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        case CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case CompareOp::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case CompareOp::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Undefined:
        default:
            return VK_COMPARE_OP_LESS;
    }
}

inline VkPipelineDepthStencilStateCreateInfo DepthStencilState(const DepthStencilState& state) {
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .pNext = nullptr };
    depth_stencil_state.depthTestEnable = state.enable_depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil_state.depthWriteEnable = state.enable_depth_write ? VK_TRUE : VK_FALSE;
    depth_stencil_state.depthCompareOp = CompareOp(state.compare_op);
    depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state.stencilTestEnable = state.enable_stencil_test ? VK_TRUE : VK_FALSE;
    depth_stencil_state.front = {};
    depth_stencil_state.back = {};
    depth_stencil_state.minDepthBounds = 0.0F;
    depth_stencil_state.maxDepthBounds = 1.0F;
    return depth_stencil_state;
}

constexpr VkAttachmentLoadOp LoadOp(LoadOp op) {
    switch (op) {
        case LoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOp::DontCare:
        case LoadOp::Undefined:
        default:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

constexpr VkAttachmentStoreOp StoreOp(StoreOp op) {
    switch (op) {
        case StoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOp::DontCare:
        case StoreOp::Undefined:
        default:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

constexpr VkClearColorValue ClearColorValue(const struct ClearColorValue& clear) {
    VkClearColorValue vk_clear;
    vk_clear.float32[0] = clear.r;
    vk_clear.float32[1] = clear.g;
    vk_clear.float32[2] = clear.b;
    vk_clear.float32[3] = clear.a;
    return vk_clear;
}

constexpr VkClearDepthStencilValue ClearDepthStencilValue(const struct ClearDepthStencilValue& clear) {
    VkClearDepthStencilValue vk_clear;
    vk_clear.depth = clear.depth;
    vk_clear.stencil = clear.stencil;
    return vk_clear;
}

constexpr VkBorderColor BorderColor(BorderColor color) {
    switch (color) {
        case BorderColor::TransparentBlack:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case BorderColor::OpaqueBlack:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case BorderColor::OpaqueWhite:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        case BorderColor::Undefined:
        default:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }
}

constexpr VkSamplerAddressMode AddressMode(AddressMode mode) {
    switch (mode) {
        case AddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case AddressMode::Undefined:
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

constexpr VkImageLayout TextureLayout(TextureLayout layout) {
    switch (layout) {
        case TextureLayout::General:
            return VK_IMAGE_LAYOUT_GENERAL;
        case TextureLayout::ColorAttachment:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureLayout::DepthStencilAttachment:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case TextureLayout::DepthStencilReadOnly:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case TextureLayout::ShaderReadOnly:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TextureLayout::TransferSrc:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TextureLayout::TransferDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case TextureLayout::Present:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case TextureLayout::Undefined:
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

constexpr enum TextureLayout ImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return TextureLayout::Undefined;
        case VK_IMAGE_LAYOUT_GENERAL:
            return TextureLayout::General;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return TextureLayout::ColorAttachment;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return TextureLayout::DepthStencilAttachment;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return TextureLayout::DepthStencilReadOnly;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return TextureLayout::ShaderReadOnly;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return TextureLayout::TransferSrc;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return TextureLayout::TransferDst;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return TextureLayout::Present;
            
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            return TextureLayout::General;
            
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
        case VK_IMAGE_LAYOUT_MAX_ENUM:
        default:
            // todo: log fatal
            std::abort();
    }
}

constexpr VkExtent2D Extent2D(struct Extent2D extent) {
    return VkExtent2D { .width = extent.width, .height = extent.height };
}

constexpr VkExtent3D Extent3D(struct Extent3D extent) {
    return VkExtent3D { .width = extent.width, .height = extent.height, .depth = extent.depth };
}

constexpr VkOffset2D Offset2D(struct Offset2D offset) {
    return VkOffset2D { .x = offset.x, .y = offset.y };
}

constexpr VkOffset3D Offset3D(struct Offset3D offset) {
    return VkOffset3D { .x = offset.x, .y = offset.y, .z = offset.z };
}

constexpr VkRect2D Rect2D(struct Rect2D rect) {
    return VkRect2D { .offset = Offset2D(rect.offset), .extent = Extent2D(rect.extent) };
}

constexpr VkViewport Viewport(const struct Viewport& viewport) {
    return VkViewport {
        .x = viewport.x, .y = viewport.y,
        .width = viewport.width, .height = viewport.height,
        .minDepth = viewport.min_depth, .maxDepth = viewport.max_depth
    };
}

constexpr VkBufferCopy BufferCopyRegion(const struct BufferCopyRegion& region) {
    return VkBufferCopy{
        .srcOffset = region.src_offset,
        .dstOffset = region.dst_offset,
        .size = region.size,
    };
}

inline std::vector<VkBufferCopy> BufferCopyRegions(std::span<struct BufferCopyRegion> regions) {
    std::vector<VkBufferCopy> vk_regions;
    vk_regions.reserve(regions.size());
    for (const auto& region : regions) {
        vk_regions.push_back(BufferCopyRegion(region));
    }
    return vk_regions;
}

constexpr VkBufferImageCopy BufferTextureCopyRegion(const struct BufferTextureCopyRegion& region, VkImageAspectFlags aspect) {
    return VkBufferImageCopy{
        .bufferOffset = region.buffer_offset,
        .bufferRowLength = region.buffer_row_length,
        .bufferImageHeight = region.buffer_image_height,
        .imageSubresource = {
            .aspectMask = aspect,
            .mipLevel = region.texture_mip_level,
            .baseArrayLayer = region.texture_base_array_layer,
            .layerCount = region.texture_layer_count,
        },
        .imageOffset = Offset3D(region.texture_offset),
        .imageExtent = Extent3D(region.texture_extent),
    };
}

inline std::vector<VkBufferImageCopy> BufferTextureCopyRegions(std::span<const struct BufferTextureCopyRegion> regions, VkImageAspectFlags aspect) {
    std::vector<VkBufferImageCopy> vk_regions;
    vk_regions.reserve(regions.size());
    for (const auto& region : regions) {
        vk_regions.push_back(BufferTextureCopyRegion(region, aspect));
    }
    return vk_regions;
}

constexpr VkImageCopy TextureCopyRegion(const struct TextureCopyRegion& region, VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) {
    return VkImageCopy {
        .srcSubresource = {
            .aspectMask = src_aspect,
            .mipLevel = region.src_subresource.mip_level,
            .baseArrayLayer = region.src_subresource.base_array_layer,
            .layerCount = region.src_subresource.layer_count,
        },
        .srcOffset = Offset3D(region.src_offset),
        .dstSubresource = {
            .aspectMask = dst_aspect,
            .mipLevel = region.dst_subresource.mip_level,
            .baseArrayLayer = region.dst_subresource.base_array_layer,
            .layerCount = region.dst_subresource.layer_count,
        },
        .dstOffset = Offset3D(region.dst_offset),
        .extent = Extent3D(region.extent),
    };
}

inline std::vector<VkImageCopy> TextureCopyRegions(std::span<const struct TextureCopyRegion> regions, VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) {
    std::vector<VkImageCopy> vk_regions;
    vk_regions.reserve(regions.size());
    for (const auto& region : regions) {
        vk_regions.push_back(TextureCopyRegion(region, src_aspect, dst_aspect));
    }
    return vk_regions;
}

constexpr VkImageBlit BlitRegion(const struct BlitRegion& region, VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) {
    return VkImageBlit {
        .srcSubresource = {
            .aspectMask = src_aspect,
            .mipLevel = region.src_subresource.mip_level,
            .baseArrayLayer = region.src_subresource.base_array_layer,
            .layerCount = region.src_subresource.layer_count,
        },
        .srcOffsets = { Offset3D(region.src_offsets[0]), Offset3D(region.src_offsets[1]) },
        .dstSubresource = {
            .aspectMask = dst_aspect,
            .mipLevel = region.dst_subresource.mip_level,
            .baseArrayLayer = region.dst_subresource.base_array_layer,
            .layerCount = region.dst_subresource.layer_count,
        },
        .dstOffsets = { Offset3D(region.dst_offsets[0]), Offset3D(region.dst_offsets[1]) },
    };
}

inline std::vector<VkImageBlit> BlitRegions(std::span<const struct BlitRegion> regions, VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) {
    std::vector<VkImageBlit> vk_regions;
    vk_regions.reserve(regions.size());
    for (const auto& region : regions) {
        vk_regions.push_back(BlitRegion(region, src_aspect, dst_aspect));
    }
    return vk_regions;
}

constexpr VkIndexType IndexType(IndexType index_type) {
    switch (index_type) {
        case IndexType::UInt16:
            return VK_INDEX_TYPE_UINT16;
        case IndexType::UInt32:
            return VK_INDEX_TYPE_UINT32;
        case IndexType::Undefined:
        default:
            // todo: log fatal
            std::abort();
    }
}

inline VkWriteDescriptorSet DescriptorWrite(VkDescriptorSet set, const struct DescriptorWrite& write, const VkDescriptorImageInfo* image_info, const VkDescriptorBufferInfo* buffer_info) {
    VkWriteDescriptorSet vk_write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pNext = nullptr };
    vk_write.dstSet = set;
    vk_write.dstBinding = write.binding;
    vk_write.dstArrayElement = 0;
    vk_write.descriptorCount = 1;
    vk_write.descriptorType = DescriptorType(write.type);
    vk_write.pImageInfo = image_info;
    vk_write.pBufferInfo = buffer_info;
    vk_write.pTexelBufferView = nullptr;
    return vk_write;
}

} // namespace convert

namespace extract {

constexpr VkImageAspectFlags ImageAspect(enum Format format) {
    switch (format) {
        case Format::R8UNorm:
        case Format::R8UNormGamma:
        case Format::R8UInt:
        case Format::RG8UNorm:
        case Format::RG8UNormGamma:
        case Format::RG8UInt:
        case Format::RGBA8UNorm:
        case Format::RGBA8UNormGamma:
        case Format::RGBA8UInt:
        case Format::BGRA8UNorm:
        case Format::BGRA8UNormGamma:
        case Format::BGRA8UInt:
        case Format::R16Float:
        case Format::R16UInt:
        case Format::RG16Float:
        case Format::RG16Int:
        case Format::RGBA16Float:
        case Format::RGBA16UInt:
        case Format::R32Float:
        case Format::R32UInt:
        case Format::RG32Float:
        case Format::RG32UInt:
        case Format::RGBA32Float:
        case Format::RGBA32UInt:
        case Format::RG11B10Float:
        case Format::RGB10A2UNorm:
            return VK_IMAGE_ASPECT_COLOR_BIT;
            
        case Format::D16UNorm:
        case Format::D32Float:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
            
        case Format::D24UNormS8UInt:
        case Format::D32FloatS8UInt:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            
        case Format::Undefined:
        default:
            // todo: log fatal
            std::abort();
    }
}

constexpr std::tuple<VkPipelineStageFlags2, VkAccessFlags2> BarrierStageAccess2(TextureLayout layout) {
    switch (layout) {
        case TextureLayout::General:
            return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 
                     VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT };
        case TextureLayout::ColorAttachment:
            return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                     VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT };
        case TextureLayout::DepthStencilAttachment:
            return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
        case TextureLayout::DepthStencilReadOnly:
            return {  VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT };
        case TextureLayout::ShaderReadOnly:
            return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                     VK_ACCESS_2_SHADER_READ_BIT };
        case TextureLayout::TransferSrc:
            return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                     VK_ACCESS_2_TRANSFER_READ_BIT };
        case TextureLayout::TransferDst:
            return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                     VK_ACCESS_2_TRANSFER_WRITE_BIT };
        case TextureLayout::Present:
        case TextureLayout::Undefined:
        default:
            return { VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE };
    }
}

} // namespace extract 

} // namespace iinf::rhi::vulkan

