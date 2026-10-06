#pragma once

#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/descriptor_set_layout.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

struct PushConstantRange {
    ShaderStage stages;
    uint32 size;
    uint32 offset;
};

struct PipelineLayoutCreateInfo {
    std::span<PushConstantRange> constant_ranges;
    std::span<const DescriptorSetLayout*> descriptor_set_layouts;
};

class PipelineLayout {
public:
    virtual ~PipelineLayout() = default;
};

} // namespace iinf::rhi
