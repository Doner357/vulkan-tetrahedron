#version 450
#define PI 3.14159265359

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
    vec3 albedo;
    float metallic;
    float roughness;
};


layout(set = 0, binding = 0, std140) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat3 normal_mat;
} ubo;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(set = 0, binding = 2, std140) uniform LightsList {
    PointLight point_lights[NUM_OF_POINT_LIGHT];
} light_list;

layout(set = 0, binding = 3, std140) uniform Material {
    float metallic;
    float roughness;
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


// ** Function for Cook-Torrance BRDF **
// Fresnel-Schlick function for F
vec3 FresnelSchlick(float cosTheta, vec3 F0);
// Fresnel-Schlick function, but take roughness into account
vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness);
// Trowbridge-Reitz GG normal distribution function for D
float DistributionGGX(vec3 N, vec3 H, float roughness);
// Schlick-GGX geometry function
float GeometrySchlickGGX(float N_dot_V, float roughness);
// GeometrySmith geometry function for G
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);


vec3 CalcPointLight(PointLight light, MaterialAttributes material, vec3 N, vec3 V, vec3 L, vec3 F0, float D);


void main() {
    // out_color = vec4(texture(tex_sampler, frag_tex_coord).rgb, 1.0);
    vec3 result = vec3(0.0);
    vec3 normal = normalize(fs_in.frag_normal);

    MaterialAttributes mat;
    mat.albedo = fs_in.frag_color;
    mat.metallic = material_attr.metallic;
    mat.roughness = material_attr.roughness;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, mat.albedo, material_attr.metallic);

    vec3 view_dir = -normalize(fs_in.frag_pos);
    for (int i = 0; i < NUM_OF_POINT_LIGHT; ++i) {
        if (light_list.point_lights[i].activated != 0) {
            vec3 light_position = vec3(ubo.view * vec4(light_list.point_lights[i].position, 1.0));
            vec3 light_dir = normalize(light_position - fs_in.frag_pos);
            float distance = distance(light_position, fs_in.frag_pos);
            result += CalcPointLight(light_list.point_lights[i], mat, normal, view_dir, light_dir, F0, distance);
        }
    }

    out_color = vec4(result, 1.0);
}


vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
	float a        = roughness * roughness;
	float a2       = a * a;
	float N_dot_H  = max(dot(N, H), 0.0);
	float N_dot_H2 = N_dot_H * N_dot_H;

	float num = a2;
	float denom = (N_dot_H2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float N_dot_V, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num   = N_dot_V;
	float denom = N_dot_V * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
	float N_dot_V = max(dot(N, V), 0.0);
	float N_dot_L = max(dot(N, L), 0.0);
	float ggx2    = GeometrySchlickGGX(N_dot_V, roughness);
	float ggx1    = GeometrySchlickGGX(N_dot_L, roughness);

	return ggx1 * ggx2;
}

vec3 CalcPointLight(PointLight light, MaterialAttributes material, vec3 N, vec3 V, vec3 L, vec3 F0, float D) {
    vec3 Lo = vec3(0.0);
    
    vec3 H = normalize(V + L);
    
    // -- attenuation --
    float attenuation = 1.0 / (light.constant + light.linear * D + light.quadratic * D * D);
    vec3 radians = light.specular * attenuation;

    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = DistributionGGX(N, H, material.roughness);
    float G = GeometrySmith(N, V, L, material.roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0000001;
    vec3 specular = numerator / denominator;

    float N_dot_L = max(dot(N, L), 0.0);
    Lo += (kD * material.albedo / PI + specular) * radians * N_dot_L;

    vec3 ambient = kD * light.ambient * F0 * material.albedo;
    ambient *= attenuation;

    return ambient + Lo;
}