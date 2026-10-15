#version 450

layout(location = 0) out vec2 out_pos;
layout(location = 1) out vec3 out_color;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.85),
    vec2(0.7,  0.8),
    vec2(-0.7,  0.8)
);

vec3 colors[3] = vec3[](
    vec3(1.00, 0.15, 0.55),
    vec3(0.00, 0.85, 1.00),
    vec3(1.00, 0.82, 0.05)
);

void main() {
    out_pos = positions[gl_VertexIndex];
    out_color = colors[gl_VertexIndex];
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
