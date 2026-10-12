#pragma once

#include <span>

#include <iinf/core/types.h>
#include <iinf/rhi/pipeline_layout.h>
#include <iinf/rhi/shader.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi {

enum class PipelineType : uint8 {
    Graphics,
    Compute
};

enum class PrimitiveTopology : uint8 {
    Undefined = 0,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan
};

enum class CullMode : uint8 {
    Undefined = 0,
    None,
    Front,
    Back,
    FrontBack
};

enum class FrontFace : uint8 {
    Undefined = 0,
    ClockWise,
    CounterClockWise
};

struct InputAssemblyState {
    PrimitiveTopology primitive_topology;
};

struct RasterizationState {
    FrontFace front_face;
    CullMode cull_mode;
    float line_width = 1.0F;
};

enum class InputRate : uint8 {
    Undefined = 0,
    PerVertex,
    PerInstance
};

struct VertexInput {
    uint32 binding;
    uint32 stride;
    InputRate input_rate;
};

struct VertexAttribute {
    uint32 binding;
    uint32 location;
    Format format;
    uint32 offset;
};

struct DepthStencilState {
    bool enable_depth_test = false; 
    bool enable_depth_write = false; 
    CompareOp compare_op = CompareOp::Less;
    bool enable_stencil_test = false;
};

enum class BlendFactor : uint8 {
    Undefined = 0,
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha
};

enum class BlendOp : uint8 {
    Undefined = 0,
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

struct BlendState {
    BlendFactor src_color;
    BlendFactor dst_color;
    BlendOp color_op;
    BlendFactor src_alpha;
    BlendFactor dst_alpha;
    BlendOp alpha_op;
};

struct GraphicsPipelineCreateInfo {
    Shader* vertex_shader;
    Shader* fragment_shader;
    PipelineLayout* pipeline_layout;

    // vertex input
    std::span<VertexInput> vertex_inputs;
    std::span<VertexAttribute> vertex_attributes;

    // input assembly
    InputAssemblyState input_assembly_state;

    // rasterization
    RasterizationState rasterization_state;

    // depth and stencil
    DepthStencilState depth_stencil_state;

    // blend
    std::span<BlendState> color_attachment_blend_states;

    // dynamic rendering format
    std::span<Format> color_attachment_formats;
    Format depth_attachment_format;
};

struct ComputePipelineCreateInfo {
    const Shader* compute_shader;
    const PipelineLayout* pipeline_layout;
};

class Pipeline {
public:
    virtual ~Pipeline() = default;

    virtual PipelineType type() const = 0;
    virtual PipelineLayout& layout() = 0;
};

} // namespace iinf::rhi
