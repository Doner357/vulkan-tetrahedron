#include "application.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <functional>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

void HelloTriangleApplication::createImGuiDescriptorPool() {

    std::array<VkDescriptorPoolSize, 11> pool_sizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // IMPORTANT!!!
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes    = pool_sizes.data();
    pool_info.maxSets       = static_cast<uint32_t>(1000);

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void HelloTriangleApplication::createImGuiRenderPass() {
    VkAttachmentDescription attachment = {};
    attachment.format         = swapchain_images_format;
    attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    if (vkCreateRenderPass(device, &info, nullptr, &imgui_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass for ImGui!");
    }
}

void HelloTriangleApplication::createImGuiFrambuffers() {
    imgui_framebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); ++i) {
        std::array<VkImageView, 1> attachments = {
            swapchain_image_views[i]
        };

        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = imgui_render_pass;
        create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        create_info.pAttachments = attachments.data();
        create_info.width = swapchain_extent.width;
        create_info.height = swapchain_extent.height;
        create_info.layers = 1;

        if (vkCreateFramebuffer(device, &create_info, nullptr, &imgui_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffers for ImGui!");
        }
    }

}

void HelloTriangleApplication::createImGuiCommandPool() {
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
    
    if (vkCreateCommandPool(device, &create_info, nullptr, &imgui_command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool for ImGui!");
    }
}

void HelloTriangleApplication::createImGuiCommandBuffers() {
    imgui_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = imgui_command_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(imgui_command_buffers.size());

    if (vkAllocateCommandBuffers(device, &alloc_info, imgui_command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

static ImFont* imgui_font = nullptr;
void HelloTriangleApplication::createImGuiContext() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = instance;
    init_info.PhysicalDevice            = physical_device;
    init_info.Device                    = device;
    init_info.QueueFamily               = queue_family_indices.graphics_family.value();
    init_info.Queue                     = graphics_queue;
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = imgui_descriptor_pool;
    init_info.RenderPass                = imgui_render_pass;
    init_info.Subpass                   = 0;
    init_info.MinImageCount             = swapchain_images.size();
    init_info.ImageCount                = swapchain_images.size();
    init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator                 = nullptr;
    init_info.CheckVkResultFn = [](VkResult err) {
        if (err == 0) return;
        std::cout << "[vulkan] Error: VkResult = " << err << "\n";
        if (err < 0) abort();
    };
    ImGui_ImplVulkan_Init(&init_info);

    std::string font_file = "fonts/Play-Regular.ttf";
    imgui_font = io.Fonts->AddFontFromFileTTF(font_file.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    if (imgui_font == nullptr) {
        std::cerr << "failed to load font file: " << font_file << '\n';
    }
}

void HelloTriangleApplication::rencordImGuiCommandBuffer(VkCommandBuffer& command_buffer, uint32_t image_index) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (imgui_font != nullptr) {
        ImGui::PushFont(imgui_font);
    }
    setupImGuiDisplay();
    if (imgui_font != nullptr) {
        ImGui::PopFont();
    }
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    VkCommandBufferBeginInfo command_begin_info{};
    command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_begin_info.flags = 0;
    command_begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(command_buffer, &command_begin_info) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording ImGui command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = imgui_render_pass;
    render_pass_info.framebuffer       = imgui_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount   = 0;
    render_pass_info.pClearValues      = nullptr;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record ImGui command buffer!");
    }
}

void HelloTriangleApplication::cleanImGuiFramebuffers() {
    for (auto framebuffer : imgui_framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

void HelloTriangleApplication::cleanupImGui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanImGuiFramebuffers();
    vkDestroyRenderPass(device, imgui_render_pass, nullptr);
    vkDestroyDescriptorPool(device, imgui_descriptor_pool, nullptr);
    vkDestroyCommandPool(device, imgui_command_pool, nullptr);
}