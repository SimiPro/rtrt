////////////////////////////////////////////////////////////////////////////////
//
// Vookoo triangle example (C) 2017 Andy Thomason
//
// This is a simple introduction to the vulkan C++ interface by way of Vookoo
// which is a layer to make creating Vulkan resources easy.
//
// In this sample we demonstrate uniforms which allow you to pass values
// to shaders that will stay the same throughout the whole draw call.
//
// Compare this file with pushConstants.cpp to see what we have done.

// Include the demo framework, vookoo (vku) for building objects and glm for maths.
// The demo framework uses GLFW to create windows.
#include <vku/vku_framework.hpp>
#include <vku/vku.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp> // for rotate()
#include <glm/gtx/rotate_vector.hpp>

struct Uniform {
    glm::vec3 spherePosition;
    glm::float32 sphereRadius;
};

int main() {
  // Initialise the GLFW framework.
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // Make a window
  const char *title = "uniforms";
  bool fullScreen = false;
  int width = 800;
  int height = 600;
  GLFWmonitor *monitor = nullptr;
  auto glfwwindow = glfwCreateWindow(width, height, title, monitor, nullptr);

  {
    // Initialise the Vookoo demo framework.
    vku::Framework fw{title};
    if (!fw.ok()) {
      std::cout << "Framework creation failed" << std::endl;
      exit(1);
    }

    // Get a device from the demo framework.
    vk::Device device = fw.device();

    // Create a window to draw into
    vku::Window window{fw.instance(), device, fw.physicalDevice(), fw.graphicsQueueFamilyIndex(), glfwwindow};
    if (!window.ok()) {
      std::cout << "Window creation failed" << std::endl;
      exit(1);
    }

    // Create two shaders, vertex and fragment. See the files uniforms.vert
    // and uniforms.frag for details.
    vku::ShaderModule vert_{device, "vert.spv"};
    vku::ShaderModule frag_{device, "frag.spv"};

    // Build a template for descriptor sets that use these shaders.
    vku::DescriptorSetLayoutMaker dslm{};
    dslm.buffer(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll, 1);
    auto descriptorSetLayout = dslm.createUnique(device);

    // Make a default pipeline layout. This shows how pointers
    // to resources are layed out.
    // 
    vku::PipelineLayoutMaker plm{};
    plm.descriptorSetLayout(*descriptorSetLayout);
    auto pipelineLayout_ = plm.createUnique(device);

    // We will use this simple vertex description.
    // It has a 2D location (x, y) and a colour (r, g, b)
    struct Vertex { glm::vec2 pos; glm::vec3 colour; };

    // This is our triangle.
    std::vector<Vertex> vertices = {};

    int num_cube_hori = 800;
    int num_cube_veri = 600;
    float step_size_hori = 2./num_cube_hori;
    float step_size_veri = 2./num_cube_veri;
    int num_cubes = num_cube_hori*num_cube_veri;
    for (int i = 0; i < num_cube_hori; i++) {
        for (int j = 0; j < num_cube_veri; j++) {
            glm::vec3 color = {(float)(i*j)/(num_cubes), (float)(i*j)/(num_cubes), (float)(i*j)/(num_cubes)};
            // v1 
            float x1 = -1. + i*step_size_hori;
            float y1 = -1. + j*step_size_veri;
            glm::vec2 v1 = {x1, y1};
            // v2 
            float x2 = -1. + i*step_size_hori;
            float y2 = -1. + (j+1)*step_size_veri;
            glm::vec2 v2 = {x2, y2};
            // v3 
            float x3 = -1. + (i+1)*step_size_hori;
            float y3 = -1. + (j+1)*step_size_veri;
            glm::vec2 v3 = {x3, y3};
            // v4 
            float x4 = -1. + (i+1)*step_size_hori;
            float y4 = -1. + j*step_size_veri;
            glm::vec2 v4 = {x4, y4};
            
            vertices.push_back({v1, color});
            vertices.push_back({v2, color});
            vertices.push_back({v3, color});

            vertices.push_back({v1, color});
            vertices.push_back({v3, color});
            vertices.push_back({v4, color});
        }
    }

    vku::HostVertexBuffer buffer(device, fw.memprops(), vertices);

    // Make a pipeline to use the vertex format and shaders.
    vku::PipelineMaker pm{(uint32_t)width, (uint32_t)height};
    pm.shader(vk::ShaderStageFlagBits::eVertex, vert_);
    pm.shader(vk::ShaderStageFlagBits::eFragment, frag_);
    pm.vertexBinding(0, (uint32_t)sizeof(Vertex));
    pm.vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, (uint32_t)offsetof(Vertex, pos));
    pm.vertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, (uint32_t)offsetof(Vertex, colour));

    // Create a pipeline using a renderPass built for our window.
    auto renderPass = window.renderPass();
    auto &cache = fw.pipelineCache();
    auto pipeline = pm.createUnique(device, cache, *pipelineLayout_, renderPass);

    // Read the pushConstants example first.
    // 
    Uniform u;
    u.spherePosition = {-3,0,-3};
    u.sphereRadius = 1;
    int frame = 0;

    // Create a single entry uniform buffer.
    // We cannot update this buffers with normal memory writes
    // because reading the buffer may happen at any time.
    auto ubo = vku::UniformBuffer{device, fw.memprops(), sizeof(Uniform)};
    int qfi = fw.graphicsQueueFamilyIndex();

    // We need to create a descriptor set to tell the shader where
    // our buffers are.
    vku::DescriptorSetMaker dsm{};
    dsm.layout(*descriptorSetLayout);
    auto sets = dsm.create(device, fw.descriptorPool());

    // Next we need to update the descriptor set with the uniform buffer.
    vku::DescriptorSetUpdater update;
    update.beginDescriptorSet(sets[0]);

    // Point the descriptor set at the storage buffer.
    update.beginBuffers(0, 0, vk::DescriptorType::eUniformBuffer);
    update.buffer(ubo.buffer(), 0, sizeof(Uniform));
    update.update(device);

    float phi = M_PI/10;
    glm::mat3x3 rot = {{cos(phi), sin(phi), 0},
                        {0,0,1},
                        {-sin(phi), cos(phi), 0}};
    // Loop waiting for the window to close.
    while (!glfwWindowShouldClose(glfwwindow)) {
        glfwPollEvents();

        //u.spherePosition = glm::rotate(u.spherePosition, glm::radians(1.0f), glm::vec3(0, 1, 0));
        // u.colour.r = std::sin(frame * 0.01f);
        float val = 2*M_PI*(frame % 1000) / 1000.0;
        //u.spherePosition = rot* glm::vec3{0, 0, -10};
        //u.sphereRadius = (frame % 1000) / 100.0;
        frame++;

        // draw one triangle.
        // Unlike helloTriangle, we generate the command buffer dynamicly
        // because it will contain different values on each frame.
        window.draw(
        device, fw.graphicsQueue(),
        [&](vk::CommandBuffer cb, int imageIndex, vk::RenderPassBeginInfo &rpbi) {
            vk::CommandBufferBeginInfo bi{};
            cb.begin(bi);
            // Instead of pushConstants() we use updateBuffer()
            // This has an effective max of about 64k.
            // Like pushConstants(), this takes a copy of the uniform buffer
            // at the time we create this command buffer.
            cb.updateBuffer(
            ubo.buffer(), 0, sizeof(u), (const void*)&u
            );
            // We may or may not need this barrier. It is probably a good precaution.
            // ubo.barrier(
            //  cb, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTopOfPipe,
            // vk::DependencyFlagBits::eByRegion,
            // vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eShaderRead, qfi, qfi
            //);

            // Unlike in the pushConstants example, we need to bind descriptor sets
            // to tell the shader where to find our buffer.
            cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout_, 0, sets[0], nullptr);

            cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
            cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

            cb.bindVertexBuffers(0, buffer.buffer(), vk::DeviceSize(0));
            cb.draw(vertices.size(), 1, 0, 0);
            cb.endRenderPass();
            cb.end();
        }
        );

      // Very crude method to prevent your GPU from overheating.
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Wait until all drawing is done and then kill the window.
    device.waitIdle();
    // The Framework and Window objects will be destroyed here.
  }
  glfwDestroyWindow(glfwwindow);
  glfwTerminate();
  return 0;
}
