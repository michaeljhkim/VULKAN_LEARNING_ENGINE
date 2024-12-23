/*
    CODE
    ENGINE (Scene)
    VULKAN
    GPU
*/

#include <iostream>
//#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <string>
#include <vector>
#include <stack>

#include <glm/glm.hpp>

#include "graphics/memory/framememory.hpp"
#include "graphics/memory/uniformmemory.hpp"

#include "graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"
#include "graphics/models/gun.hpp"
#include "graphics/models/sphere.hpp"
#include "graphics/models/box.hpp"
#include "graphics/models/plane.hpp"
#include "graphics/models/brickwall.hpp"

#include "graphics/objects/model.hpp"

#include "graphics/rendering/shader.hpp"
#include "graphics/rendering/texture.hpp"
#include "graphics/rendering/light.hpp"
#include "graphics/rendering/cubemap.hpp"
#include "graphics/rendering/text.hpp"

#include "physics/environment.hpp"
#include "physics/collisionmesh.hpp"

#include "io/keyboard.hpp"
#include "io/mouse.hpp"
#include "io/joystick.hpp"
#include "io/camera.hpp"

#include "algorithms/states.hpp"
#include "algorithms/ray.hpp"
#include "algorithms/bounds.hpp"

#include "scene.hpp"

Scene scene;

void processInput(double dt);
void renderScene(Shader shader);

Camera cam;

//Joystick mainJ(0);

double dt = 0.0f; // tme btwn frames
double lastFrame = 0.0f; // time of last frame

Sphere sphere(10);
//Cube cube(10);
Lamp lamp(4);
Brickwall wall;

std::string Shader::defaultDirectory = "assets/shaders";

/*
void TEST() {
    globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
          .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();

    std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<LveBuffer>(
            lveDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uboBuffers[i]->map();
    }

    auto globalSetLayout =
        LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorBufferInfo();
        auto imageInfo = uboBuffers[i]->descriptorImageInfo();
        LveDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(0, &imageInfo)
            .build(globalDescriptorSets[i]);
    }
}
*/

int main() {
    std::cout << "Hello, Vulkan!" << std::endl;

    // construct scene
    scene = Scene(4, 4, "Vulkan Game Engine", 2560, 1440);
    // test if SDL2 successfully created and started window
    if (!scene.init()) {
        std::cout << "Could not open window" << std::endl;
        scene.cleanup();
        return -1;
    }

    // set camera
    scene.cameras.push_back(&cam);
    scene.activeCamera = 0;

    // SHADERS===============================
    Shader::loadIntoDefault("defaultHead.gh");

    Shader shader(true, "instanced/instanced.vs", "object.fs");
    Shader boxShader(false, "instanced/box.vs", "instanced/box.fs");
    Shader dirShadowShader(false, "shadows/dirSpotShadow.vs", "shadows/dirShadow.fs");
    Shader spotShadowShader(false, "shadows/dirSpotShadow.vs", "shadows/pointSpotShadow.fs");
    Shader pointShadowShader(false, "shadows/pointShadow.vs", "shadows/pointSpotShadow.fs",  "shadows/pointShadow.gs");

    Shader::clearDefault();

    // FONTS===============================
    TextRenderer font(32);
    if (!scene.registerFont(&font, "comic", "assets/fonts/comic.ttf")) {
        std::cout << "Could not load font" << std::endl;
    }

    // MODELS==============================
    scene.registerModel(&lamp);
    scene.registerModel(&wall);
    scene.registerModel(&sphere);
    //scene.registerModel(&cube);

    Box box;
    box.init();

    // load all model data
    scene.loadModels();

    // LIGHTS==============================

    // directional light
    //BoundingRegion dirLight_BR(glm::vec3(-20.0f, -20.0f, 0.5f), glm::vec3(20.0f, 20.0f, 50.0f));
    DirLight dirLight(glm::vec3(-0.2f, -0.9f, -0.2f),
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
        glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
        glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
        BoundingRegion(glm::vec3(-20.0f, -20.0f, 0.5f), glm::vec3(20.0f, 20.0f, 50.0f)) );
    scene.dirLight = &dirLight;

    // point lights
    float move_up = 5.0f;
    glm::vec3 pointLightPositions[] = {
        glm::vec3(1.0f, move_up+ 1.0f, 0.0f),
        glm::vec3(0.0f, move_up+ 15.0f,  0.0f),
        glm::vec3(-4.0f, move_up+ 2.0f, -12.0f),
        glm::vec3(0.0f, move_up+ 0.0f, -3.0f)
    };

    glm::vec4 ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    glm::vec4 diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    glm::vec4 specular = glm::vec4(1.0f);
    float k0 = 1.0f;
    float k1 = 0.0014f;
    float k2 = 0.000007f;

    PointLight pointLights[4];

    for (GLuint i = 0; i < 1; i++) {
        pointLights[i] = PointLight(
            pointLightPositions[i],
            k0, k1, k2,
            ambient, diffuse, specular,
            0.5f, 50.0f
        );
        // create physical model for each lamp
        scene.generateInstance(lamp.id, glm::vec3(10.0f, 0.25f, 10.0f), 0.25f, pointLightPositions[i]);
        // add lamp to scene's light source
        scene.pointLights.push_back(&pointLights[i]);
        // activate lamp in scene
        States::activateIndex(&scene.activePointLights, i);
    }

    // spot light
    SpotLight spotLight(
        //glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
        cam.cameraPos, cam.cameraFront, cam.cameraUp,
        glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(20.0f)),
        1.0f, 0.0014f, 0.000007f,
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec4(1.0f),
        0.1f, 100.0f
    );
    scene.spotLights.push_back(&spotLight);
    //scene.activeSpotLights = 1; // 0b00000001

    /*
    scene.generateInstance(cube.id, glm::vec3(20.0f, 0.1f, 20.0f), 100.0f, glm::vec3(0.0f, -3.0f, 0.0f));
    glm::vec3 cubePositions[] = {
        { 1.0f, 3.0f, -5.0f },
        { -7.25f, 2.1f, 1.5f },
        { -15.0f, 2.55f, 9.0f },
        { 4.0f, -3.5f, 5.0f },
        { 2.8f, 1.9f, -6.2f },
        { 3.5f, 6.3f, -1.0f },
        { -3.4f, 10.9f, -5.5f },
        { 0.0f, 11.0f, 0.0f },
        { 0.0f, 5.0f, 0.0f }
    };
    for (unsigned int i = 0; i < 9; i++) {
        scene.generateInstance(cube.id, glm::vec3(0.5f), 1.0f, cubePositions[i]);
    }
    */

    // instantiate the brickwall plane
    scene.generateInstance(wall.id, glm::vec3(1.0f), 1.0f, { 0.0f, 0.0f, 2.0f }, { -1.0f, glm::pi<float>(), 0.0f });

    //scene.generateInstance(sphere.id, glm::vec3(0.1f), 1.0f, cam.cameraPos);

    // instantiate instances
    scene.initInstances();

    // finish preparations (octree, etc)
    scene.prepare(box, { shader });

    // joystick recognition
    /*mainJ.update();
    if (mainJ.isPresent()) {
        std::cout << mainJ.getName() << " is present." << std::endl;
    }*/

    //scene.variableLog["time"] = (double)0.0;

    scene.defaultFBO.bind(); // bind default framebuffer

    while (!scene.shouldClose()) {
        // calculate currentTime (in seconds)
        Uint64 counter = SDL_GetPerformanceCounter();
        Uint64 frequency = SDL_GetPerformanceFrequency();
        double currentTime = static_cast<double>(counter) / frequency;

        // calculate dt (delta time)
        dt = currentTime - lastFrame;
        lastFrame = currentTime;

        //scene.variableLog["time"] += dt;
        //scene.variableLog["fps"] = 1 / dt;

        // update screen values
        scene.update();

        // process input
        processInput(dt);

        // activate the directional light's FBO

        // remove launch objects if too far
        for (int i = 0; i < sphere.currentNumInstances; i++) {
            if (glm::length(cam.cameraPos - sphere.instances[i]->pos) > 250.0f) {
                scene.markForDeletion(sphere.instances[i]->instanceId);
            }
        }

        //// render scene to dirlight FBO
        //dirLight.shadowFBO.activate();
        //scene.renderDirLightShader(dirShadowShader);
        //renderScene(dirShadowShader);

        //// render scene to point light FBOs
        //for (unsigned int i = 0, len = scene.pointLights.size(); i < len; i++) {
        //    if (States::isIndexActive(&scene.activePointLights, i)) {
        //        scene.pointLights[i]->shadowFBO.activate();
        //        scene.renderPointLightShader(pointShadowShader, i);
        //        renderScene(pointShadowShader);
        //    }
        //}

        //// render scene to spot light FBOs
        //for (unsigned int i = 0, len = scene.spotLights.size(); i < len; i++) {
        //    if (States::isIndexActive(&scene.activeSpotLights, i)) {
        //        scene.spotLights[i]->shadowFBO.activate();
        //        scene.renderSpotLightShader(spotShadowShader, i);
        //        renderScene(spotShadowShader);
        //    }
        //}

        // render scene normally
        scene.defaultFBO.activate();
        scene.renderShader(shader);
        renderScene(shader);
        
        // render boxes
        scene.renderShader(boxShader, false);
        box.render(boxShader);

        // send new frame to window
        scene.newFrame(box);    //THIS FUNCTION CALL IS WHERE SPHERE HAS BEEN CAUSING SEGFAULTS - CHECK MORE LATER IF NEEDE

        // clear instances that have been marked for deletion
        scene.clearDeadInstances();
        //std::cout << "Do we even get here?" << std::endl;
    }

    // clean up objects
    scene.cleanup();
    return 0;
}

void renderScene(Shader shader) {
    if (sphere.currentNumInstances > 0) {
        scene.renderInstances(sphere.id, shader, dt);
    }

    //scene.renderInstances(cube.id, shader, dt);

    scene.renderInstances(lamp.id, shader, dt);

    scene.renderInstances(wall.id, shader, dt);
}

void launchItem(float dt) {
    RigidBody* rb = scene.generateInstance(sphere.id, glm::vec3(0.1f), 1.0f, cam.cameraPos);
    if (rb) {
        // instance generated successfully
        rb->transferEnergy(25.0f, cam.cameraFront);
        rb->applyAcceleration(Environment::gravitationalAcceleration);
    }
}

void emitRay() {
    Ray r(cam.cameraPos, cam.cameraFront);

    float tmin = std::numeric_limits<float>::max();
    BoundingRegion* intersected = scene.octree->checkCollisionsRay(r, tmin);
    if (intersected) {
        std::cout << "Hits " << intersected->instance->instanceId << " at t = " << tmin << std::endl;
        scene.markForDeletion(intersected->instance->instanceId);
    }
    else {
        std::cout << "No hit" << std::endl;
    }
}

void processInput(double dt) {
    // process input with cameras
    scene.processInput(dt);

    // close window
    if (Keyboard::key(SDL_SCANCODE_ESCAPE)) {
        scene.setShouldClose(true);
    }

    // update flash light
    if (States::isIndexActive(&scene.activeSpotLights, 0)) {
        scene.spotLights[0]->position = scene.getActiveCamera()->cameraPos;
        scene.spotLights[0]->direction = scene.getActiveCamera()->cameraFront;
        scene.spotLights[0]->up = scene.getActiveCamera()->cameraUp;
        scene.spotLights[0]->updateMatrices();
    }

    if (Keyboard::keyWentDown(SDL_SCANCODE_1)) {
        States::toggleIndex(&scene.activeSpotLights, 0); // toggle spot light
    }

    // launch sphere
    if (Keyboard::keyWentDown(SDL_SCANCODE_F)) {
        launchItem(dt);
    }

    // emit ray
    if (Mouse::buttonWentDown(SDL_BUTTON_LEFT)) {
        emitRay();
    }
    // determine if each lamp should be toggled
    for (int i = 0; i < 4; i++) {
        if (Keyboard::keyWentDown(SDL_GetScancodeFromKey(SDLK_1 + i)) ) {
            //States::toggleIndex(&scene.activePointLights, i);
        }
    }
}