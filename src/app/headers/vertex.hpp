#ifndef LVK_VERTEX_HPP
#define LVK_VERTEX_HPP

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec3 color;

    // Same as providing some parameters of glVertexAttribIPointer
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding   = 0;                              // The index of the binding in the array of bindings.
        binding_description.stride    = sizeof(Vertex);                 // The number of bytes from one entry to the next
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    // Could be per vertex or per instance

        return binding_description;
    }

    // Same as providing some parameters of glVertexAttribIPointer
    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 6> attribute_descriptions{};
        
        // For vertex position
        attribute_descriptions[0].binding  = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset   = offsetof(Vertex, pos);
        // For vertex color
        attribute_descriptions[1].binding  = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset   = offsetof(Vertex, normal);
        // For texture coordinates
        attribute_descriptions[2].binding  = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset   = offsetof(Vertex, tex_coord);
        // For tangent
        attribute_descriptions[3].binding  = 0;
        attribute_descriptions[3].location = 3;
        attribute_descriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[3].offset   = offsetof(Vertex, tangent);
        // For bitangent
        attribute_descriptions[4].binding  = 0;
        attribute_descriptions[4].location = 4;
        attribute_descriptions[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[4].offset   = offsetof(Vertex, bitangent);
        // For Color
        attribute_descriptions[5].binding  = 0;
        attribute_descriptions[5].location = 5;
        attribute_descriptions[5].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[5].offset   = offsetof(Vertex, color);

        return attribute_descriptions;
    }


};


inline glm::mat3 calculateCounterClockwiseTbn(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3) {
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    glm::vec3 edge1 = p2 - p1;
    glm::vec3 edge2 = p3 - p1;

    normal = glm::normalize(glm::cross(edge1, edge2));

    glm::vec2 delta_uv1 = uv2 - uv1;
    glm::vec2 delta_uv2 = uv3 - uv1;

    float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

    tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
    tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
    tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);

    bitangent.x = f * (-delta_uv2.x * edge1.x - delta_uv1.x * edge2.x);
    bitangent.y = f * (-delta_uv2.x * edge1.y - delta_uv1.x * edge2.y);
    bitangent.z = f * (-delta_uv2.x * edge1.z - delta_uv1.x * edge2.z);

    return glm::mat3(normal, tangent, bitangent);
}

#endif