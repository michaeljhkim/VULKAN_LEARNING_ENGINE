#ifndef SCENE_H
#define SCENE_H

//#include <glad/glad.h>
//#include <GLFW/glfw3.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

/*
#include <json/allocator.h>
#include <json/assertions.h>
#include <json/config.h>
#include <json/forwards.h>
#include <json/json_features.h>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/version.h>
#include <json/writer.h>
*/

#include "graphics/memory/framememory.hpp"
#include "graphics/memory/uniformmemory.hpp"

#include "graphics/models/box.hpp"

//#include "graphics/objects/model.h"
#include "graphics/model.hpp"

#include "graphics/rendering/light.hpp"
#include "graphics/rendering/shader.hpp"
#include "graphics/rendering/text.hpp"

#include "io/camera.hpp"
#include "io/keyboard.hpp"
#include "io/mouse.hpp"

#include "algorithms/states.hpp"
#include "algorithms/avl.hpp"
#include "algorithms/octree.hpp"
#include "algorithms/trie.hpp"

// forward declarations
namespace Octree {
    class node;
}

class Model;

/*
    Scene class
    - ties together the many functions in the program (rendering, physics, collision, etc)
*/

class Scene {
public:
    // tries to store models/instances
    avl* models;
    trie::Trie<RigidBody*> instances;
    /*
    std::vector<Vertex> globalVertices;
    std::vector<uint32_t> globalIndices;
	std::vector<std::unique_ptr<VkDrawIndexedIndirectCommand>> globalIndirectCommands;

    std::unique_ptr<VulkanBuffer> globalVertexBuffer;
	std::unique_ptr<VulkanBuffer> globalIndexBuffer;
	uint32_t vertexCount;
  	bool hasIndexBuffer = false;
	uint32_t indexCount;

    // to be called after constructor
    void createGlobalBuffers();
    */

    // list of instances that should be deleted
    std::vector<RigidBody*> instancesToDelete;

    // pointer to root node in octree
    std::unique_ptr<Octree::node> octree;

    // map for logged variables
    //Jsoncpp::json variableLog;

    // freetype library
    FT_Library ft;
    avl* fonts;

    FramebufferObject defaultFBO;

    UBO::UBO lightUBO;

    /*
        callbacks
    */
    
    // window resize
    static void framebufferSizeCallback(SDL_Window* window, int width, int height);

    /*
        constructors
    */
    
    // default
    Scene();

    // set with values
    Scene(int SDL2VersionMajor, int SDL2VersionMinor, const char* title, unsigned int scrWidth, unsigned int scrHeight);

    /*
        initialization
    */

    // to be called after constructor
    bool init();

    // register a font family
    bool registerFont(TextRenderer* tr, std::string name, std::string path);

    // to be called after instances have been generated/registered
    void prepare(Box &box, std::vector<Shader> shaders);

    /*
        main loop methods
    */
    //static SDL_Event event;
    // process input
    void processInput(float dt);

    // update screen before each frame
    void update();

    // update inputs each frame
    void updateInput();

    // update screen after frame
    void newFrame(Box &box);

    // set uniform shader varaibles (lighting, etc)
    void renderShader(Shader shader, bool applyLighting = true);

    // set uniform shader variables for directional light render
    void renderDirLightShader(Shader shader);

    // set uniform shader variables for point light render
    void renderPointLightShader(Shader shader, unsigned int idx);

    // set uniform shader variables for spot light render
    void renderSpotLightShader(Shader shader, unsigned int idx);

    // render specified model's instances
    void renderInstances(std::string modelId, Shader shader, float dt);

    // render text
    void renderText(std::string font, Shader shader, std::string text, float x, float y, glm::vec2 scale, glm::vec3 color);

    /*
        cleanup method
    */

    // called after main loop
    void cleanup();

    /*
        accessors
    */

    // determine if window should close
    bool shouldClose();

    // get current active camera in scene
    Camera* getActiveCamera();

    /*
        modifiers
    */

    // set if the window should close
    void setShouldClose(bool shouldClose);

    // set window background color
    void setWindowColor(float r, float g, float b, float a);

    /*
        Model/instance methods
    */

    // register model into model trie
    void registerModel(Model* model);

    // generate instance of specified model with physical parameters
    RigidBody* generateInstance(std::string modelId,
        glm::vec3 size = glm::vec3(1.0f), 
        float mass = 1.0f, 
        glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 rot = glm::vec3(0.0f) );

    // initialize model instances
    void initInstances();

    // load model data
    void loadModels();

    // delete instance
    void removeInstance(std::string instanceId);

    // mark instance for deletion
    void markForDeletion(std::string instanceId);

    // clear all instances marked for deletion
    void clearDeadInstances();

    // current instance id
    std::string currentId;

    // generate next instance id
    std::string generateId();

    /*
        lights
    */

    // list of point lights
    unsigned int noPointLights;
    std::vector<PointLight*> pointLights;
    // acts as array of switches for point lights
    unsigned int activePointLights;

    // list of spot lights
    unsigned int noSpotLights;
    std::vector<SpotLight*> spotLights;
    // acts as array of switches for spot lights
    unsigned int activeSpotLights;

    // direction light
    DirLight* dirLight;
    // switch for directional light
    bool dirLightActive;

    /*
        camera
    */

    // list of cameras
    std::vector<Camera*> cameras;
    // index of active camera
    unsigned int activeCamera;

    // camera position/matrices
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 textProjection;
    glm::vec3 cameraPos;

protected:
	void createVertexSparseBuffers();
	void createIndexSparseBuffers();

    // window object
    SDL_Window* window;

    VulkanWindow vulkanWindow{1920, 1080, "Vulkan test"};
    VulkanDevice vulkanDevice{vulkanWindow};

    // window vals
    const char* title;
    static unsigned int scrWidth;
    static unsigned int scrHeight;

    float bg[4]; // background color

    // SDL2 info
    int SDL2VersionMajor;
    int SDL2VersionMinor;
};

#endif