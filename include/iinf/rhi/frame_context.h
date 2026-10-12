#pragma once

#include <iinf/core/types.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/texture.h>

namespace iinf::rhi {

class FrameContext {
public:
    virtual ~FrameContext() = default;

    virtual CommandBuffer& command_buffer() const = 0;
    virtual Texture& present_texture() const = 0;
};

} // namespace iinf::rhi

