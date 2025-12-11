#include "application.hpp"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstddef>
#include <array>
#include <limits>


static bool show_grid = false;
static int operation_mode = 0;
static int operation_selection = 0;
static int operation_space_selection = 0;
static int auto_rotation_axis = 0;
static int auto_rotation_movement = 0;
static int auto_rotation_space = 0;
static float auto_rotation_speed = 1.0f;

static std::array<bool, 2> enable_light_list = { true, true };

void HelloTriangleApplication::setupImGuiDisplay() {
    
    // Adjustment
    ImGui::SetNextWindowSize(ImVec2(350, 0));
    ImGuiStyle& style = ImGui::GetStyle();
    style.ItemSpacing.y = 7.0f;
    ImGui::Begin("Operation", nullptr, ImGuiWindowFlags_NoResize);

    ImVec2 region_extent = ImGui::GetContentRegionAvail();
    ImGui::PushItemWidth(region_extent.x);
    
    if (ImGui::Button("Reset Camera", ImVec2(region_extent.x, 0))) {
        camera.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
        camera.setLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.setUp(glm::vec3(0.0f, 1.0f, 0.0f));
        camera.checkUpSide();
    }

    ImGui::Checkbox("Show Bad Grid", &show_grid);

    ImGui::SeparatorText("Background Color");
    
    ImGui::ColorEdit3("##", glm::value_ptr(clear_color));

    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::PushID("Tetrahedron Editor");
    if (ImGui::CollapsingHeader("Tetrahedron Editor")) {
        
        if (ImGui::Button("Reset Object")) {
            tetrahedron_model = glm::mat4(1.0f);
            tetra_depth = 0;
            vertex_buffer_need_update = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Trace Object")) {
            camera.setLookAt(glm::vec3(tetrahedron_model[3]));
        }
        
        ImGui::SliderInt("##", reinterpret_cast<int*>(&tetra_depth), 0, 10, "Depth=%d");
        if (tetra_depth != pre_depth) {
            vertex_buffer_need_update = true;
        }
        

        ImGui::Separator();
        ImGui::Text("Operation Mode");
        ImGui::RadioButton("Manual", &operation_mode, 0);
        ImGui::RadioButton("Auto", &operation_mode, 1);

        //////////////////////////////////////////////
        ImGui::PushID("Manual");

        ImGui::BeginDisabled(operation_mode != 0);

        ImGui::Spacing();
        ImGui::SeparatorText("Manual");
        ImGui::Text("Transformation Operation");
        ImGui::RadioButton("Translate", &operation_selection, 0); ImGui::SameLine();
        ImGui::RadioButton("Rotation", &operation_selection, 1); ImGui::SameLine();
        ImGui::RadioButton("Scale", &operation_selection, 2);
        
        if (operation_selection != 2) {
            ImGui::Text("Transformation Space");
            ImGui::RadioButton("Local", &operation_space_selection, 0); ImGui::SameLine();
            ImGui::RadioButton("World", &operation_space_selection, 1);
        }
        ImGui::EndDisabled();

        ImGui::PopID();

        //////////////////////////////////////////////
        ImGui::PushID("Auto");

        ImGui::BeginDisabled(operation_mode != 1);

        ImGui::Spacing();
        ImGui::SeparatorText("Auto");
        ImGui::Text("Auto Rotation");
        ImGui::RadioButton("X", &auto_rotation_axis, 0); ImGui::SameLine();
        ImGui::RadioButton("Y", &auto_rotation_axis, 1); ImGui::SameLine();
        ImGui::RadioButton("Z", &auto_rotation_axis, 2);

        ImGui::Text("Movement");
        ImGui::RadioButton("Stop", &auto_rotation_movement, 0);
        ImGui::RadioButton("Clockwise", &auto_rotation_movement, 1);
        ImGui::RadioButton("Counter-Clockwise", &auto_rotation_movement, 2);

        ImGui::PushID("Space");
        ImGui::Text("Space");
        ImGui::RadioButton("Local", &auto_rotation_space, 0); ImGui::SameLine();
        ImGui::RadioButton("World", &auto_rotation_space, 1);
        ImGui::PopID();

        ImGui::Text("Speed");
        ImGui::SliderFloat("##", &auto_rotation_speed, 0.0f, 100.0f);

        ImGui::EndDisabled();

        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Text("Material Attributes");
        ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);

        ImGui::PopID();

        ImGui::PushItemWidth(region_extent.x);
    }
    ImGui::PopID();

    
    ImGui::PopItemWidth();

    ImGui::PushID("Lights");
    if (ImGui::CollapsingHeader("Lights")) {

        ImGui::PushID("Light 1");
        {
            PointLight& light1 = points_lights[0];
            ImGui::SeparatorText("Light 0");
            ImGui::Checkbox("Enable", &enable_light_list[0]);
            if (enable_light_list[0]) {
                light1.activated = 1;
            }
            else {
                light1.activated = 0;
            }
            ImGui::DragFloat3("Position", glm::value_ptr(light1.position), 0.025f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            ImGui::DragFloat3("Color", glm::value_ptr(light1.specular), 0.0025f, 0.0f, 1.0f);            
        }
        ImGui::PopID();

        ImGui::Spacing();

        ImGui::PushID("Light 2");
        {
            PointLight& light2 = points_lights[1];
            ImGui::SeparatorText("Light 1");
            ImGui::Checkbox("Enable", &enable_light_list[1]);
            if (enable_light_list[1]) {
                light2.activated = 1;
            }
            else {
                light2.activated = 0;
            }
            ImGui::DragFloat3("Position", glm::value_ptr(light2.position), 0.025f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            ImGui::DragFloat3("Color", glm::value_ptr(light2.specular), 0.0025f, 0.0f, 1.0f);            
        }
        ImGui::PopID();

    }
    ImGui::PopID();

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::Text("Close the application");
    if (ImGui::Button("Exit", ImVec2(region_extent.x, 0))) {
        glfwSetWindowShouldClose(window, true);
    }
    ImGui::End();


    // ImGuizmo Editor
    ImGuizmo::BeginFrame();

    ImGuizmo::MODE mat_operation_space = ImGuizmo::LOCAL;
    ImGuizmo::OPERATION mat_operation = ImGuizmo::TRANSLATE;

    switch (operation_space_selection) {
    case 0:
        mat_operation_space = ImGuizmo::LOCAL;
        break;
    case 1:
        mat_operation_space = ImGuizmo::WORLD;
        break;
    default:
        break;
    }

    switch (operation_selection) {
    case 0:
        mat_operation = ImGuizmo::TRANSLATE;
        break;
    case 1:
        mat_operation = ImGuizmo::ROTATE;
        break;
    case 2:
        mat_operation = ImGuizmo::SCALE;
        break;
    default:
        break;
    }
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    if (operation_mode == 0) {
        ImGuizmo::Manipulate(
            glm::value_ptr(camera.getViewMat()),
            glm::value_ptr(camera.getProjectionMat()),
            mat_operation,
            mat_operation_space,
            glm::value_ptr(tetrahedron_model)
        );        
    }
    
    glm::mat4 grid_trans = glm::mat4(1.0f);
    if (show_grid) {
        ImGuizmo::DrawGrid(
            glm::value_ptr(camera.getViewMat()),
            glm::value_ptr(camera.getProjectionMat()),
            glm::value_ptr(grid_trans),
            200.0f
        );
    }

    if (operation_mode == 1) {
        glm::vec3 rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
        switch (auto_rotation_axis) {
        case 0:
            rotation_axis = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        
        case 1:
            rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        
        case 2:
            rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        
        default:
            break;
        }

        glm::vec4 tetra_translation = tetrahedron_model[3];
        switch (auto_rotation_movement) {
        case 0: // stop
            break;

        case 1: // clockwise
            if (auto_rotation_space == 0) {
                tetrahedron_model = glm::rotate(tetrahedron_model * glm::mat4(1.0f), -delta_time * auto_rotation_speed, rotation_axis);
            }
            else if (auto_rotation_space == 1) {
                tetrahedron_model = glm::rotate(glm::mat4(1.0f), -delta_time * auto_rotation_speed, rotation_axis) * tetrahedron_model;
            }
            tetrahedron_model[3] = tetra_translation;
            break;

        case 2: // counter-clockwise
            if (auto_rotation_space == 0) {
                tetrahedron_model = glm::rotate(tetrahedron_model * glm::mat4(1.0f), delta_time * auto_rotation_speed, rotation_axis);
            }
            else if (auto_rotation_space == 1) {
                tetrahedron_model = glm::rotate(glm::mat4(1.0f), delta_time * auto_rotation_speed, rotation_axis) * tetrahedron_model;
            }
            tetrahedron_model[3] = tetra_translation;
            break;
        
        default:
            break;
        }        
    }



    // Analyze
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(350, 55));
    ImGui::Begin("Measurements", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::Text("Application average %.3f ms / frame (%u FPS)", 1000.0f / fps, fps);
    ImGui::End();

}