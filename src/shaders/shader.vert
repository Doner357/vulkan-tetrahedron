#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat3 normal_mat;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;
layout(location = 5) in vec3 in_color;

layout(location = 0) out VS_OUT {
    vec3 frag_pos;
    vec3 frag_normal;
    vec2 frag_tex_coord;
    mat3 frag_tbn;
    mat3 frag_inverse_tbn;
    vec3 frag_color;
} vs_out;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0);

    vec3 T   = normalize(ubo.normal_mat * in_tangent);
    vec3 B   = normalize(ubo.normal_mat * in_bitangent);
    vec3 N   = normalize(ubo.normal_mat * in_normal);
    mat3 TBN = mat3(T, B, N);

    vs_out.frag_pos         = vec3(ubo.view * ubo.model * vec4(in_position, 1.0));
    vs_out.frag_normal      = N;
    vs_out.frag_tex_coord   = in_tex_coord;
    vs_out.frag_tbn         = TBN;
    vs_out.frag_inverse_tbn = transpose(TBN);
    vs_out.frag_color       = in_color;
}