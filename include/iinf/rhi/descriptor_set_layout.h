#pragma once

#include <vector>

#include <iinf/rhi/descriptor.h>

namespace iinf::rhi {

struct DescriptorSetLayoutCreateInfo {
    std::vector<Descriptor> descriptors;    
};

class DescriptorSetLayout {
public:
    virtual ~DescriptorSetLayout() = default;
};

} // namespace iinf::rhi
