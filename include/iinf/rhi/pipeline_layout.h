#pragma once

#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/descriptor_set_layout.h>
#include <iinf/rhi/types.h>
#include <vector>

namespace iinf::rhi {

struct PushConstantRange {
    ShaderStage stages;
    uint32 size;
    uint32 offset;
};

struct PipelineLayoutCreateInfo {
    std::span<PushConstantRange> constant_ranges;
    std::span<DescriptorSetLayout*> descriptor_set_layouts;
};

class PipelineLayout {
public:
    virtual ~PipelineLayout() = default;

    virtual std::vector<PushConstantRange>& constant_ranges() = 0;
    virtual std::vector<DescriptorSetLayout*>& descriptor_set_layouts() = 0;
};

} // namespace iinf::rhi
