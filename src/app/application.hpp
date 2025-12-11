#ifndef SCHOOL_VULKAN_APP
#define SCHOOL_VULKAN_APP

// GLFW will include its own definitions and automatically
// load the Vulkan header with it.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stb/stb_image.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"


#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <functional>
#include <map>

#include "headers/camera.hpp"
#include "headers/vertex.hpp"
#include "headers/tetrahedron.hpp"
#include "headers/tacothreadpool.hpp"

static const std::string WINDOW_NAME = reinterpret_cast<const char*>(u8"S11159023-賴弘翔");

// Window's width and height
const uint32_t WIDTH  = 800;
const uint32_t HEIGHT = 600;

// How many in-flight frames
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

// Manully load Create Debug Utils Messenger function since it's a extension function.
inline VkResult CreateDebugUtilsMessengerEXT(
        VkInstance                                instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks*              pAllocator,
        VkDebugUtilsMessengerEXT*                 pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT"
    );
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
// Manully load Destroy Debug Utils Messenger function
inline void DestroyDebugUtilsMessengerEXT(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool isComplete() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;   // Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images) 
    std::vector<VkSurfaceFormatKHR> formats;        // Surface formats (pixel format, color space)
    std::vector<VkPresentModeKHR>   present_modes;  // Available presentation modes
};


// Structure for model-view-projection matrix
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 normal_mat;   // Thought this is mat3 actually, need to round to mat4 for alignment
};

struct ObjectMaterial {
    alignas(4) float metallic;
    alignas(4) float roughness;
};

struct PointLight {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(4)  float     constant;
    alignas(4)  float     linear;
    alignas(4)  float     quadratic;
    alignas(4)  uint32_t  activated;
};

struct LightUniformBufferObject {
    std::array<PointLight, 2> lights;
};


struct ObjectMaterialUniformObject {
    ObjectMaterial material;
};


class HelloTriangleApplication {
    public:
        void run() {
            initWindow();
            initData();
            initLightData();
            initVulkan();
            initImGui();
            initCamera();
            mainLoop();
            cleanup();
        }

    private:
        // Members
        GLFWwindow*              window;          // GLFW provided window

        VkInstance               instance;        // The instance of Vulkan library
        VkDebugUtilsMessengerEXT debug_messenger; // Manually-handled debug messenger
        VkSurfaceKHR             surface;         // Surface extension for window, which is optional

        // For Physical Device
        VkPhysicalDevice         physical_device = VK_NULL_HANDLE; // Physical device like graphic card

        // For Logical Device
        VkDevice                 device;          // Logical Device. Important!
        QueueFamilyIndices       queue_family_indices;
        VkQueue                  graphics_queue;  // Queues along with logical device (graphics)
        VkQueue                  present_queue;   // Queues along with logical device (surface presentation)

        // For swapchain
        VkSwapchainKHR           swapchain;                 // Swapchain handle
        std::vector<VkImage>     swapchain_images;          // The images in swapchain
        VkFormat                 swapchain_images_format;   // The format of swapchain images
        VkExtent2D               swapchain_extent;          // The extent info of swapchain images
        std::vector<VkImageView> swapchain_image_views;     // Image View objects for swapchain images

        // For render passes & descriptor set layout & pipeline
        VkRenderPass          render_pass;
        VkPipelineLayout      pipeline_layout;
        VkDescriptorSetLayout descriptor_set_layout;
        VkPipeline            graphics_pipeline;
        glm::vec3 clear_color = glm::vec3(0.1f, 0.1f, 0.1f);

        // For command pool
        VkCommandPool   command_pool;

        // For command buffer
        std::vector<VkCommandBuffer> command_buffers;

        // For the swapchain's framebuffers
        std::vector<VkFramebuffer> swapchain_framebuffers;

        // For sync objects
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence>     inflight_fences;

        // Addtional check to guarantee the recreation of swapchain is needed.
        bool framebuffer_resized = false;

        // Current rendering frame
        uint32_t current_frame = 0;

        // Vertices for rectangle
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        std::vector<Vertex> vertices;
        // Indices for rectangle
        VkBuffer index_buffer;
        VkDeviceMemory index_memory;
        const std::vector<uint16_t> indices {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
        bool vertex_buffer_need_update = false;

        // Uniform buffer for transformation matrix
        // Use multiple buffer to prevent the in-flight frame read the
        // buffer which is still read by previous frame's draw command.
        std::vector<VkBuffer>       uniform_buffers;
        std::vector<VkDeviceMemory> uniform_buffers_memory;
        std::vector<void*>          uniform_buffers_mapped;

        std::vector<VkBuffer>       lights_uniform_buffers;
        std::vector<VkDeviceMemory> lights_uniform_memory;
        std::vector<void*>          lights_uniform_mapped;
        std::array<PointLight, 2>   points_lights;

        std::vector<VkBuffer>       material_uniform_buffers;
        std::vector<VkDeviceMemory> material_uniform_memory;
        std::vector<void*>          material_uniform_mapped;
        ObjectMaterial              material;

        // Descriptor Pool for uniform buffers
        VkDescriptorPool descriptor_pool;
        std::vector<VkDescriptorSet> descriptor_sets;

        // Image objects
        VkImage        texture_image;
        VkDeviceMemory texture_image_memroy;
        VkImageView    texture_image_view;
        VkSampler      texture_sampler;

        // Depth attachment
        VkImage        depth_image;
        VkDeviceMemory depth_image_memory;
        VkImageView    depth_image_view;

        // Lights handler

        // Camera
        taco::ArcballCamera camera;


        // Time
        float current_time = 0;
        float previous_time = 0;
        float delta_time = 0;

        // FPS
        uint32_t fps = 0;
        uint32_t fps_counter = 0;
        float fps_pass_time = 0;


        // Initialize GLFW window
        void initWindow();
        // GLFW call back for resize window
        static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
        static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
        // glfw: whenever the mouse scroll wheel scrolls, this callback is called
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        static void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


        void initCamera();


        uint32_t tetra_depth = 0;
        uint32_t pre_depth = 0;
        glm::mat4 tetrahedron_model = glm::mat4(1.0f);
        std::unordered_map<uint32_t, std::vector<Vertex>> tetrahedrons_vertices;
        void initData();
        void initLightData();


        // ImGUI
        VkDescriptorPool imgui_descriptor_pool;
        VkRenderPass imgui_render_pass;
        VkCommandPool imgui_command_pool;
        std::vector<VkCommandBuffer> imgui_command_buffers;
        std::vector<VkFramebuffer> imgui_framebuffers;
        void createImGuiDescriptorPool();
        void createImGuiRenderPass();
        void createImGuiCommandPool();
        void createImGuiCommandBuffers();
        void createImGuiFrambuffers();
        void createImGuiContext();
        void initImGui() {
            createImGuiDescriptorPool();
            createImGuiRenderPass();
            createImGuiFrambuffers();
            createImGuiCommandPool();
            createImGuiCommandBuffers();
            createImGuiContext();
        }
        void rencordImGuiCommandBuffer(VkCommandBuffer& command_buffer, uint32_t image_index);
        void cleanImGuiFramebuffers();
        void cleanupImGui();

        void setupImGuiDisplay();


        void initVulkan();


        void mainLoop();

        void handleEvent();

        // Update current time and delta time
        void updateClock();

        // Update the content of fps
        void updateFps();


        // Cleanup objects for swapchain
        void cleanupSwapchain();

        void cleanup();


        //////////////////////////////////////////////////////////////////
        // Instance, Debug Utils Messenger, and also window surface
        //////////////////////////////////////////////////////////////////
        void createInstance();

        std::vector<const char*> getRequiredExtensions();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);

        void setupDebugMessenger();

        // Check if all the required layers are available
        bool checkValidationLayerSupport();

        // Customized Debug Callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
            VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
            VkDebugUtilsMessageTypeFlagsEXT             message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallback_date,
            void*                                       pUser_date
        );

        // Get window's surface handle from glfw
        void createSurface();


        //////////////////////////////////////////////////////////////////
        // Physical device
        //////////////////////////////////////////////////////////////////
        void pickPhysicalDevice();

        // Check if the device is suitable
        bool isDeviceSuitable(VkPhysicalDevice device);

        // Check if the given device support required extensions.
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // Find suitable queue family
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);


        //////////////////////////////////////////////////////////////////
        // Logical device
        //////////////////////////////////////////////////////////////////
        void createLogicalDevice();


        //////////////////////////////////////////////////////////////////
        // Swapchain (Swap Chain)
        //////////////////////////////////////////////////////////////////
        void createSwapchain();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        void createImageViews();


        //////////////////////////////////////////////////////////////////
        // Render Passes
        //////////////////////////////////////////////////////////////////
        void createRenderPass();


        //////////////////////////////////////////////////////////////////
        // Dexcriptor Layout for Uniform Buffer
        //////////////////////////////////////////////////////////////////
        void createDescriptorLayout();


        //////////////////////////////////////////////////////////////////
        // Graphics Pipelines
        //////////////////////////////////////////////////////////////////
        void createGraphicsPipeline();

        // Read \SPIR-V shader byte codes
        static std::vector<char> readFile(const std::string& filename);

        VkShaderModule createShaderModule(const std::vector<char>& code);


        //////////////////////////////////////////////////////////////////
        // Swapchain's Framebuffers
        //////////////////////////////////////////////////////////////////
        void createFramebuffers();


        //////////////////////////////////////////////////////////////////
        // Command Pool
        //////////////////////////////////////////////////////////////////
        void createCommandPool();


        //////////////////////////////////////////////////////////////////
        // Buffer Utilities
        //////////////////////////////////////////////////////////////////
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory
        );

        uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

        void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);


        //////////////////////////////////////////////////////////////////
        // Depth Attachment Resources
        //////////////////////////////////////////////////////////////////
        void createDepthResorces();

        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags
            features
        );

        VkFormat findDepthFormat();

        bool hasStencilComponent(VkFormat format);


        //////////////////////////////////////////////////////////////////
        // Texture Image (Depend Command buffer and so on buffer pool)
        //////////////////////////////////////////////////////////////////
        void createTextureImage();

        void createImage(
            uint32_t width, uint32_t height, VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& image_memory
        );

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint16_t height);


        //////////////////////////////////////////////////////////////////
        // Image view
        //////////////////////////////////////////////////////////////////
        void createTextureImageView();

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);


        //////////////////////////////////////////////////////////////////
        // Texture Sampler
        //////////////////////////////////////////////////////////////////
        void createTextureSampler();


        //////////////////////////////////////////////////////////////////
        // Vertex buffer
        //////////////////////////////////////////////////////////////////
        void createVertexBuffer();

        void updateVertexBuffer();


        //////////////////////////////////////////////////////////////////
        // Index buffer
        //////////////////////////////////////////////////////////////////
        void createIndexBuffer();


        //////////////////////////////////////////////////////////////////
        // Uniform buffers
        //////////////////////////////////////////////////////////////////
        void createUniformBuffers();


        //////////////////////////////////////////////////////////////////
        // Descriptor Pool
        //////////////////////////////////////////////////////////////////
        void createDescriptorPool();


        //////////////////////////////////////////////////////////////////
        // Descriptor Sets
        //////////////////////////////////////////////////////////////////
        void createDescriptorSets();


        //////////////////////////////////////////////////////////////////
        // Command Buffer
        //////////////////////////////////////////////////////////////////
        void createCommandBuffers();

        // Note that if the command buffer was already recorded once, then a call to
        // vkBeginCommandBuffer will implicitly reset it (since ). It's not possible to append
        // commands to a buffer at a later time.
        void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);

        VkCommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(VkCommandBuffer command_buffer);


        //////////////////////////////////////////////////////////////////
        // Sync Objects
        //////////////////////////////////////////////////////////////////
        void createSyncObjects();


        //////////////////////////////////////////////////////////////////
        // Draw Frame
        //////////////////////////////////////////////////////////////////
        void drawFrame();


        //////////////////////////////////////////////////////////////////
        // Uniform buffer updating
        //////////////////////////////////////////////////////////////////
        void updateUniformBuffer(uint32_t current_frame);


        //////////////////////////////////////////////////////////////////
        // Swapchain Recreation
        //////////////////////////////////////////////////////////////////
        void recreateSwapchain();
};

#endif // SCHOOL_VULKAN_APP