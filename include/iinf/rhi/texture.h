#pragma once

#include <iinf/core/types.h>
#include <iinf/rhi/sampler.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

enum class MipMode : uint8 {
    Undefined = 0,
    None,
    Fullchain,
};

struct TextureCreateInfo {
    Extent2D extent;
    Format format;
    TextureUsage texture_usages;
    MemoryUsage memory_usage;
    uint32 array_layers = 1;
    MipMode mip_mode = MipMode::None;
};

class Texture {
public:
    virtual ~Texture() = default;

    virtual TextureUsage usage() const = 0;
    virtual TextureLayout layout() const = 0;
    virtual Extent2D extent() const = 0;
    virtual Format format() const = 0;
    virtual uint32 mip_levels() const = 0;
    virtual uint32 array_layers() const = 0;
};

} // namespace iinf::rhi
