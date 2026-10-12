#pragma once

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

enum class BufferUsage : uint8 {
    Undefined = 0,
    TransferSrc = 1 << 0,
    TransferDst = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    Vertex = 1 << 4,
    Index = 1 << 5,
    Indirect = 1 << 6
};

constexpr BufferUsage operator|(BufferUsage left, BufferUsage right) {
    uint8 usages = static_cast<uint8>(left) | static_cast<uint8>(right);
    return static_cast<BufferUsage>(usages);
};

constexpr BufferUsage operator&(BufferUsage left, BufferUsage right) {
    uint8 usages = static_cast<uint8>(left) & static_cast<uint8>(right);
    return static_cast<BufferUsage>(usages);
};

struct BufferCreateInfo {
    uint32 size;
    BufferUsage buffer_usage;
    MemoryUsage memory_usage;
};

class Buffer {
public:
    virtual ~Buffer() = default;

    virtual void* Map() = 0;
    virtual void Unmap() = 0;

    virtual uint32 size() const = 0;
    virtual BufferUsage usage() const = 0;
};

} // namespace iinf::rhi
