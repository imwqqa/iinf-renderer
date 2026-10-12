#pragma once

#include <span>
#include <variant>
#include <vector>

#include <iinf/core/types.h>
#include <iinf/rhi/buffer.h>
#include <iinf/rhi/combined_texture_sampler.h>
#include <iinf/rhi/descriptor.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi {

using DescriptorData = std::variant<const Sampler*, const Texture*, const CombinedTextureSampler*, const Buffer*>;

struct DescriptorWrite {
    uint32 binding;
    DescriptorType type; 
    DescriptorData data;
};

class DescriptorSet {
public:
    virtual ~DescriptorSet() = default;

    virtual void Update(std::span<DescriptorWrite> writes) = 0;
};

} // namespace iinf::rhi
