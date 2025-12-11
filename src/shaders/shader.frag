#version 450

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    uint  activated;
};
#define NUM_OF_POINT_LIGHT 2

struct MaterialAttributes {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat3 normal_mat;
} ubo;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(set = 0, binding = 2) uniform LightsList {
    PointLight point_lights[NUM_OF_POINT_LIGHT];
} light_list;

layout(set = 0, binding = 3) uniform Material {
    float metallic;
    float shininess;
} material_attr;


layout(location = 0) in VS_OUT {
    vec3 frag_pos;
    vec3 frag_normal;
    vec2 frag_tex_coord;
    mat3 frag_tbn;
    mat3 frag_inverse_tbn;
    vec3 frag_color;
} fs_in;

layout(location = 0) out vec4 out_color;

vec3 CalcPointLight(PointLight light, MaterialAttributes material, vec3 N, vec3 V, vec3 L, float D);

void main() {
    // out_color = vec4(texture(tex_sampler, frag_tex_coord).rgb, 1.0);
    vec3 result = vec3(0.0);
    vec3 normal = normalize(fs_in.frag_normal);

    MaterialAttributes mat;
    mat.ambient   = fs_in.frag_color * (1.2 - material_attr.metallic);
    mat.diffuse   = fs_in.frag_color * (1.0 - material_attr.metallic);
    mat.specular  = mix(vec3(0.2), fs_in.frag_color, material_attr.metallic);
    mat.shininess = material_attr.shininess;

    vec3 view_dir = -normalize(fs_in.frag_pos);
    for (int i = 0; i < NUM_OF_POINT_LIGHT; ++i) {
        if (light_list.point_lights[i].activated != 0) {
            vec3 light_position = vec3(ubo.view * vec4(light_list.point_lights[i].position, 1.0));
            vec3 light_dir = normalize(light_position - fs_in.frag_pos);
            float distance = distance(light_position, fs_in.frag_pos);
            result += CalcPointLight(light_list.point_lights[i], mat, normal, view_dir, light_dir, distance);
        }
    }

    out_color = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, MaterialAttributes material, vec3 N, vec3 V, vec3 L, float D) {
    vec3 H = normalize(V + L);

    // -- ambient --
    vec3 ambient = light.ambient * material.ambient;

    // -- diffuse --
    float diff = max(dot(L, N), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    // -- specular --
    float spec = pow(max(dot(N, H), 0.0), material.shininess) * (diff <= 0.0 ? 0.0 : 1.0);
    vec3 specular = light.specular * spec * material.specular;

    // -- attenuation --
    float attenuation = 1.0 / (light.constant + light.linear * D + light.quadratic * D * D);

    return (ambient + diffuse + specular) * attenuation;
}