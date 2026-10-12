#pragma once

#include <iinf/rhi/sampler.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi {

struct CombinedTextureSampler {
    Texture* texture;
    Sampler* sampler;
};

} // namespace iinf::rhi
