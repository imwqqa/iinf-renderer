#pragma once

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

enum class AddressMode : uint8 {
    Undefined = 0,
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum class BorderColor : uint8 {
    Undefined = 0,
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite
};

enum class MipmapMode : uint8 {
    Undefined = 0,
    Nearest,
    Linear
};

struct SamplerCreateInfo {
    Filter min_filter;
    Filter mag_filter;
    MipmapFilter mip_filter;
    AddressMode address_mode_u;
    AddressMode address_mode_v;
    AddressMode address_mode_w;
    BorderColor border_color = BorderColor::TransparentBlack;
    float max_anisotropy = 1.0F;
    bool enable_compare = false;
    CompareOp compare_op = CompareOp::Less;
};
    
class Sampler {
public:
    virtual ~Sampler() = default;
};

} // namespace iinf::rhi
