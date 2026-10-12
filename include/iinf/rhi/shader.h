#pragma once

#include <span>
#include <string>

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

struct ShaderCreateInfo {
    std::string entry_point = "main";
    std::span<uint32> bytecode;
};
    
class Shader {
public:
    virtual ~Shader() = default;

    virtual std::string& entry_point() = 0;
};

} // namespace iinf::rhi
