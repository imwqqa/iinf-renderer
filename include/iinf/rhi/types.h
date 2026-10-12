#pragma once

#include <iinf/core/types.h>

namespace iinf::rhi {

enum class MemoryUsage : uint8 {
    Undefined = 0,
    GPUOnly,
    CPUToGPU,
    GPUToCPU
};

enum class TextureUsage : uint8 {
    Undefined = 0,
    Sampled = 1 << 0,
    Storage = 1 << 1,
    TransferSrc = 1 << 2,
    TransferDst = 1 << 3,
    ColorAttachment = 1 << 4,
    DepthStencilAttachment = 1 << 5
};

constexpr TextureUsage operator|(TextureUsage left, TextureUsage right) {
    uint8 usages = static_cast<uint8>(left) | static_cast<uint8>(right);
    return static_cast<TextureUsage>(usages);
};

constexpr TextureUsage operator&(TextureUsage left, TextureUsage right) {
    uint8 usages = static_cast<uint8>(left) & static_cast<uint8>(right);
    return static_cast<TextureUsage>(usages);
};

enum class Format : uint8 {
    Undefined = 0,

    R8UNorm,
    R8UNormGamma,
    R8UInt,
    RG8UNorm,
    RG8UNormGamma,
    RG8UInt,
    RGBA8UNorm,
    RGBA8UNormGamma,
    RGBA8UInt,
    BGRA8UNorm,
    BGRA8UNormGamma,
    BGRA8UInt,

    R16Float,
    R16UInt,
    RG16Float,
    RG16Int,
    RGBA16Float,
    RGBA16UInt,

    R32Float,
    R32UInt,
    RG32Float,
    RG32UInt,
    RGBA32Float,
    RGBA32UInt,

    RG11B10Float,
    RGB10A2UNorm,

    D16UNorm,
    D32Float,
    D24UNormS8UInt,
    D32FloatS8UInt,
};

enum class ShaderStage : uint8 {
    Undefined = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2,
    All = Vertex | Fragment | Compute
};

constexpr ShaderStage operator|(ShaderStage left, ShaderStage right) {
    uint8 stages = static_cast<uint8>(left) | static_cast<uint8>(right);
    return static_cast<ShaderStage>(stages);
};

constexpr ShaderStage operator&(ShaderStage left, ShaderStage right) {
    uint8 stages = static_cast<uint8>(left) & static_cast<uint8>(right);
    return static_cast<ShaderStage>(stages);
};

enum class IndexType : uint8 {
    Undefined = 0,
    UInt16,
    UInt32
};

enum class FilterMode : uint8 {
    Undefined = 0,
    Nearest,
    Linear,
};

enum class CompareOp : uint8 {
    Undefined = 0,
    Never,
    Always,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

} // namespace iinf::rhi
