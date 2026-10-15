// glslc -fshader-stage=frag triangle.frag -o triangle.frag.spv
#version 450

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(pow(in_color, vec3(2.2)), 1.0);
}
