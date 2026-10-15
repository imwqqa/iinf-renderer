#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <span>
#include <vector>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <iinf/platform/platform.h>
#include <iinf/platform/types.h>
#include <iinf/platform/window.h>
#include <iinf/rhi/command_buffer.h>
#include <iinf/rhi/frame_context.h>
#include <iinf/rhi/pipeline.h>
#include <iinf/rhi/render_context.h>
#include <iinf/rhi/types.h>

namespace platform = iinf::platform;
namespace rhi = iinf::rhi;

namespace {

std::vector<uint32_t> load_spv(const char* path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    auto size = f.tellg();
    f.seekg(0);
    std::vector<uint32_t> code(size / 4);
    f.read(reinterpret_cast<char*>(code.data()), size);
    return code;
}

} // namespace

int main() {
    if (!platform::Init()) {
        return 1;
    }

    platform::WindowCreateInfo window_info = {
        .title = "IINF Renderer",
        .width = 1920,
        .height = 1080,
        .mode = platform::WindowMode::Windowed,
        .flags = platform::WindowFlags::MatchPixelDensity | platform::WindowFlags::Resizable
    };

    auto window = platform::Window::Create(window_info);
    auto extent = window->extent();

    rhi::SwapchainCreateInfo swapchain_info = {
        .extent = extent,
        .format = rhi::Format::BGRA8UNormGamma,
        .image_count = 3,
        .enable_vsync = true,
    };
    std::vector<rhi::SwapchainCreateInfo> swapchain_infos = { swapchain_info };
    auto render_context = rhi::RenderContext::Create({
        .window_handle  = window->native_handle().window_handle,
        .display_handle = window->native_handle().display_handle,
        .swapchain_infos = swapchain_infos,
        .inflight_count  = 3,
    });

    auto vert_code = load_spv("shaders\\triangle.vert.spv");
    auto frag_code = load_spv("shaders\\triangle.frag.spv");
    auto vert = render_context->CreateShader({ .entry_point = "main", .bytecode = vert_code });
    auto frag = render_context->CreateShader({ .entry_point = "main", .bytecode = frag_code });

    auto pipeline_layout = render_context->CreatePipelineLayout({
        .constant_ranges = {},
        .descriptor_set_layouts = {},
    });

    rhi::Format color_format = rhi::Format::BGRA8UNormGamma;
    rhi::BlendState no_blend = {};
    auto pipeline = render_context->CreateGraphicsPipeline({
        .vertex_shader = vert.get(),
        .fragment_shader = frag.get(),
        .pipeline_layout = pipeline_layout.get(),

        .vertex_inputs = {},
        .vertex_attributes = {},

        .input_assembly_state = { .primitive_topology = rhi::PrimitiveTopology::TriangleList },
        .rasterization_state = {
            .front_face = rhi::FrontFace::CounterClockWise,
            .cull_mode  = rhi::CullMode::None,
            .line_width = 1.0F,
        },
        .depth_stencil_state = { .enable_depth_test = false },
        .color_attachment_blend_states = std::span<rhi::BlendState>{ &no_blend, 1 },
        .color_attachment_formats = std::span<rhi::Format>{ &color_format, 1 },
        .depth_attachment_format = rhi::Format::Undefined,
    });

    bool is_close_requested = false;
    while (!is_close_requested) {
        window->PollEvents([&](const platform::WindowEvent& event) {
            if (event.type == platform::WindowEventType::CloseRequested) {
                is_close_requested = true;
            }
        });

        rhi::FrameContext& frame_context = render_context->BeginFrame(0);
        rhi::CommandBuffer& cmd = frame_context.command_buffer();

        rhi::ColorAttachment color_attachment = {
            .texture = &frame_context.present_texture(),
            .load_op = rhi::LoadOp::Clear,
            .store_op = rhi::StoreOp::Store,
            .clear_value = { 0.92F, 0.93F, 0.94F, 1.0F },
        };
        cmd.Transition(frame_context.present_texture(), rhi::TextureLayout::ColorAttachment);
        cmd.BeginRendering({
            .color_attachments = std::span<rhi::ColorAttachment>{ &color_attachment, 1 },
            .depth_stencil_attachment = {},
            .render_area = { .offset = { 0, 0 }, .extent = extent },
        });

        cmd.BindPipeline(*pipeline);
        cmd.SetViewport({
            .x = 0.0F, .y = 0.0F,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .min_depth = 0.0F, .max_depth = 1.0F,
        });
        cmd.SetScissor({ .offset = { 0, 0 }, .extent = extent });
        cmd.DrawUnindexed(3, 1, 0, 0);

        cmd.EndRendering();
        cmd.Transition(frame_context.present_texture(), rhi::TextureLayout::Present);
        render_context->EndFrame(0);
    }

    render_context->WaitIdle();

    platform::Exit();
}
