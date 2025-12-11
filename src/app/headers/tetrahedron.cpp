#include "tetrahedron.hpp"

#include <vector>

#include <glm/glm.hpp>

#include "vertex.hpp"

namespace taco {
    void subdivideTetrahedron(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, int depth, std::vector<Vertex>& vertices) {
        if (depth == 0) {
            /*
            glm::vec2 tex_coord1(0.5f, 1.0f); // Up
            glm::vec2 tex_coord2(0.0f, 0.0f); // Down left
            glm::vec2 tex_coord3(1.0f, 0.0f); // Down right
            */

            float weight = 0.5;
            glm::vec3 color1 = glm::vec3(1.0f, 1.0f, 1.0f) * weight;
            glm::vec3 color2 = glm::vec3(0.0f, 1.0f, 0.0f) * weight;
            glm::vec3 color3 = glm::vec3(0.0f, 0.0f, 1.0f) * weight;
            glm::vec3 color4 = glm::vec3(1.0f, 0.0f, 0.0f) * weight;

            glm::vec2 f1_tex_coord1(0.234f, 0.9967f); // Up
            glm::vec2 f1_tex_coord2(  0.0f, 0.7233f); // Down left
            glm::vec2 f1_tex_coord3(0.494f, 0.71f);   // Down right

            glm::vec2 f2_tex_coord1(0.75f , 1.0f);    // Up
            glm::vec2 f2_tex_coord2(0.986f, 0.6533f); // Down left
            glm::vec2 f2_tex_coord3(0.504f, 0.6667f); // Down right

            glm::vec2 f3_tex_coord1(0.236f, 0.49f);   // Up
            glm::vec2 f3_tex_coord2(0.002f, 0.1733f); // Down left
            glm::vec2 f3_tex_coord3(0.5f  , 0.1667f); // Down right

            glm::vec2 f4_tex_coord1(0.732f, 0.5067f); // Up
            glm::vec2 f4_tex_coord2(0.5f  , 0.1333f); // Down left
            glm::vec2 f4_tex_coord3(0.994f, 0.1233f); // Down right

            /*
            glm::vec3 normal1 = glm::normalize(glm::cross(v3 - v2, v1 - v2));
            glm::vec3 normal2 = glm::normalize(glm::cross(v4 - v1, v2 - v1));
            glm::vec3 normal3 = glm::normalize(glm::cross(v1 - v4, v3 - v4));
            glm::vec3 normal4 = glm::normalize(glm::cross(v4 - v2, v3 - v2));
            */

            glm::mat3 tbn1 = calculateCounterClockwiseTbn(v1, v2, v3, f1_tex_coord1, f1_tex_coord2, f1_tex_coord3);
            glm::mat3 tbn2 = calculateCounterClockwiseTbn(v2, v1, v4, f2_tex_coord2, f2_tex_coord3, f2_tex_coord1);
            glm::mat3 tbn3 = calculateCounterClockwiseTbn(v3, v4, v1, f3_tex_coord2, f3_tex_coord1, f3_tex_coord3);
            glm::mat3 tbn4 = calculateCounterClockwiseTbn(v3, v2, v4, f4_tex_coord2, f4_tex_coord3, f4_tex_coord1);


            vertices.push_back({v1, tbn1[0], f1_tex_coord1, tbn1[1], tbn1[2], color1});
            vertices.push_back({v2, tbn1[0], f1_tex_coord2, tbn1[1], tbn1[2], color1});
            vertices.push_back({v3, tbn1[0], f1_tex_coord3, tbn1[1], tbn1[2], color1});

            vertices.push_back({v2, tbn2[0], f2_tex_coord2, tbn2[1], tbn2[2], color2});
            vertices.push_back({v1, tbn2[0], f2_tex_coord3, tbn2[1], tbn2[2], color2});
            vertices.push_back({v4, tbn2[0], f2_tex_coord1, tbn2[1], tbn2[2], color2});

            vertices.push_back({v3, tbn3[0], f3_tex_coord2, tbn3[1], tbn3[2], color3});
            vertices.push_back({v4, tbn3[0], f3_tex_coord1, tbn3[1], tbn3[2], color3});
            vertices.push_back({v1, tbn3[0], f3_tex_coord3, tbn3[1], tbn3[2], color3});

            vertices.push_back({v3, tbn4[0], f4_tex_coord2, tbn4[1], tbn4[2], color4});
            vertices.push_back({v2, tbn4[0], f4_tex_coord3, tbn4[1], tbn4[2], color4});
            vertices.push_back({v4, tbn4[0], f4_tex_coord1, tbn4[1], tbn4[2], color4});
        } else {
            // Midpoints of edges
            glm::vec3 v12 = (v1 + v2) * 0.5f;
            glm::vec3 v13 = (v1 + v3) * 0.5f;
            glm::vec3 v14 = (v1 + v4) * 0.5f;
            glm::vec3 v23 = (v2 + v3) * 0.5f;
            glm::vec3 v24 = (v2 + v4) * 0.5f;
            glm::vec3 v34 = (v3 + v4) * 0.5f;

            subdivideTetrahedron(v1, v12, v13, v14, depth - 1, vertices);
            subdivideTetrahedron(v12, v2, v23, v24, depth - 1, vertices);
            subdivideTetrahedron(v13, v23, v3, v34, depth - 1, vertices);
            subdivideTetrahedron(v14, v24, v34, v4, depth - 1, vertices);
        }
    }
} // lvk
