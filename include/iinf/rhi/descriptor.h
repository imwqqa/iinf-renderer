#pragma once

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

enum class DescriptorType : uint8 {
    Undefined = 0,
    Sampler,
    CombinedTextureSampler,
    SampledTexture,
    StorageTexture,
    UniformBuffer,
    StorageBuffer
};

struct Descriptor {
    uint32 binding;
    DescriptorType type; 
    ShaderStage stages;
};

} // namespace iinf::rhi
