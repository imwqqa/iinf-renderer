#pragma once

#include <span>
#include <string_view>

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

struct ShaderCreateInfo {
    ShaderStage stages;
    std::string_view entry_point = "main";
    std::span<const uint32> bytecode;
};
    
class Shader {
public:
    virtual ~Shader() = default;

    virtual ShaderStage stages() const = 0;
    virtual std::string_view entry_point() const = 0;
};

} // namespace iinf::rhi
