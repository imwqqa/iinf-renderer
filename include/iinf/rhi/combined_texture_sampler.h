#pragma once

#include <iinf/rhi/sampler.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi {

struct CombinedTextureSampler {
    const Texture* texture;
    const Sampler* sampler;
};

} // namespace iinf::rhi
