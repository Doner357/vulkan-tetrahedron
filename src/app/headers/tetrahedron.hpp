#ifndef LVK_SIERPINSKI_HPP
#define LVK_SIERPINSKI_HPP

#include <vector>

#include <glm/glm.hpp>

#include "vertex.hpp"

namespace taco {

    void subdivideTetrahedron(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, int depth, std::vector<Vertex>& vertices);

    inline std::vector<Vertex> generateFractalTetrahedron(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, int depth) {
        std::vector<Vertex> verticies;
        subdivideTetrahedron(v1, v2, v3, v4, depth, verticies);
        return verticies;
    }

} // lvk

#endif // LVK_SIERPINSKI_HPP