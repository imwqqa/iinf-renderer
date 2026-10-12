#pragma once

#include <cstddef>
#include <cstdint>

namespace iinf {

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using usize = std::size_t;

struct Extent2D {
    uint32 width = 0;
    uint32 height = 0;
};

struct Extent3D {
    uint32 width = 0;
    uint32 height = 0;
    uint32 depth = 1;
};

struct Offset2D {
    int32 x = 0;
    int32 y = 0;
};

struct Offset3D {
    int32 x = 0;
    int32 y = 0;
    int32 z = 0;
};

struct Rect2D {
    Offset2D offset;
    Extent2D extent;
};

struct Rect3D {
    Offset3D offset;
    Extent3D extent;
};

} // namespace iinf
