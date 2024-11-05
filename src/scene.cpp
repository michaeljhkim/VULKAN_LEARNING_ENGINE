#include "scene.h"

#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 2

unsigned int Scene::scrWidth = 0;
unsigned int Scene::scrHeight = 0;


//for getting every single input user can make in SDL
SDL_Event event;

/*
    callbacks
*/

// window resize
void Scene::framebufferSizeCallback(SDL_Window* window, int width, int height) {
    glViewport(0, 0, width, height);
    // update variables
    Scene::scrWidth = width;
    Scene::scrHeight = height;
    //SDL_GetWindowSize(window, &width, &height);
}

/*
    constructors
*/

// default
Scene::Scene() 
    : currentId("aaaaaaaa"), lightUBO(0) {}

// set with values
Scene::Scene(int SDL2VersionMajor, int SDL2VersionMinor,
    const char* title, unsigned int scrWidth, unsigned int scrHeight)
    : SDL2VersionMajor(SDL2VersionMajor), SDL2VersionMinor(SDL2VersionMinor), // GLFW version
    title(title), // window title
    // default indices/vals
    activeCamera(-1), 
    activePointLights(0), activeSpotLights(0),
    currentId("aaaaaaaa"), lightUBO(0) {
    
    // window dimensions
    Scene::scrWidth = scrWidth;
    Scene::scrHeight = scrHeight;
    defaultFBO = FramebufferObject(scrWidth, scrHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // window color
    setWindowColor(0.1f, 0.15f, 0.15f, 1.0f);
}

/*
    initialization
*/

// to be called after constructor
bool Scene::init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL failed to initialize: " << SDL_GetError() << std::endl;
        exit(1);
    }


    // set version
    // Set OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 1);

#ifdef __APPLE__
    SDL_GL_SetAttribute(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // initialize window
    //window = glfwCreateWindow(scrWidth, scrHeight, title, NULL, NULL);
    // Create SDL window with OpenGL context
    window = SDL_CreateWindow(title,    /* Title of the SDL window */
                SDL_WINDOWPOS_UNDEFINED,    /* Position x of the window */
                SDL_WINDOWPOS_UNDEFINED,    /* Position y of the window */
                scrWidth, scrHeight,     /* Width and Height of the window in pixels */
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);     /* Additional flag(s) */

    // not created
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }


    // Create an OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Set the context to be current
    SDL_GL_MakeCurrent(window, gl_context);

    // set GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // setup screen
    glViewport(0, 0, scrWidth, scrHeight);

    /*
        callbacks
    */
    /*
    // key pressed
    glfwSetKeyCallback(window, Keyboard::keyCallback);
    // cursor moved
    glfwSetCursorPosCallback(window, Mouse::cursorPosCallback);
    // mouse btn pressed
    glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
    // mouse scroll
    glfwSetScrollCallback(window, Mouse::mouseWheelCallback);
    */

    /*
        set rendering parameters
    */

    // depth testing
    glEnable(GL_DEPTH_TEST); // doesn't show vertices not visible to camera (back of object)
    // blending for text textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // stencil testing
    glEnable(GL_STENCIL_TEST);
    // keep fragments if either stencil or depth fails, replace if both pass
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Enable v-sync
    SDL_GL_SetSwapInterval(1);  // 1 enables v-sync

    // Disable cursor
    SDL_ShowCursor(SDL_DISABLE);

    /*
        init model/instance trees
    */
    models = avl_createEmptyRoot(strkeycmp);
    instances = trie::Trie<RigidBody*>(trie::ascii_lowercase);

    /*
        init octree
    */
    octree = new Octree::node(BoundingRegion(glm::vec3(-16.0f), glm::vec3(16.0f)));

    /*
        initialize freetype library
    */
    if (FT_Init_FreeType(&ft)) {
        std::cout << "Could not init FreeType library" << std::endl;
        return false;
    }
    fonts = avl_createEmptyRoot(strkeycmp);

    return true;
}

// register a font family
bool Scene::registerFont(TextRenderer* tr, std::string name, std::string path) {
    if (tr->loadFont(ft, path)) {
        fonts = avl_insert(fonts, (void*)name.c_str(), tr);
        return true;
    }
    else {
        return false;
    }
}

// to be called after instances have been generated/registered
void Scene::prepare(Box& box, std::vector<Shader> shaders) {
    // close FT library
    FT_Done_FreeType(ft);
    // process current instances
    octree->update(box);

    // setup lighting UBO
    lightUBO = UBO::UBO(0, {
        UBO::newStruct({ // dir light
            UBO::Type::VEC3,

            UBO::Type::VEC4,
            UBO::Type::VEC4,
            UBO::Type::VEC4,

            UBO::Type::SCALAR,

            UBO::newColMat(4, 4)
        }),

        UBO::Type::SCALAR, // no point lights
        UBO::newArray(MAX_POINT_LIGHTS, UBO::newStruct({
            UBO::Type::VEC3,

            UBO::Type::VEC4,
            UBO::Type::VEC4,
            UBO::Type::VEC4,

            UBO::Type::SCALAR,
            UBO::Type::SCALAR,
            UBO::Type::SCALAR,

            UBO::Type::SCALAR
        })),

        UBO::Type::SCALAR, // no spot lights
        UBO::newArray(MAX_SPOT_LIGHTS, UBO::newStruct({
            UBO::Type::VEC3,
            UBO::Type::VEC3,

            UBO::Type::SCALAR,
            UBO::Type::SCALAR,

            UBO::Type::VEC4,
            UBO::Type::VEC4,
            UBO::Type::VEC4,

            UBO::Type::SCALAR,
            UBO::Type::SCALAR,
            UBO::Type::SCALAR,

            UBO::Type::SCALAR,
            UBO::Type::SCALAR,

            UBO::newColMat(4, 4)
        }))
    });

    // attach the UBO to specified shaders
    for (Shader s : shaders) {
        lightUBO.attachToShader(s, "Lights");
    }

    // setup memory
    lightUBO.generate();
    lightUBO.bind();
    lightUBO.initNullData(GL_STATIC_DRAW);
    lightUBO.bindRange();

    // write initial values
    lightUBO.startWrite();

    // directional light
    lightUBO.writeElement<glm::vec3>(&dirLight->direction);
    lightUBO.writeElement<glm::vec4>(&dirLight->ambient);
    lightUBO.writeElement<glm::vec4>(&dirLight->diffuse);
    lightUBO.writeElement<glm::vec4>(&dirLight->specular);
    lightUBO.writeElement<float>(&dirLight->br.max.z);
    lightUBO.writeArrayContainer<glm::mat4, glm::vec4>(&dirLight->lightSpaceMatrix, 4);

    // point lights
    noPointLights = std::min<unsigned int>(pointLights.size(), MAX_POINT_LIGHTS);
    lightUBO.writeElement<unsigned int>(&noPointLights);
    unsigned int i = 0;
    for (; i < noPointLights; i++) {
        lightUBO.writeElement<glm::vec3>(&pointLights[i]->position);
        lightUBO.writeElement<glm::vec4>(&pointLights[i]->ambient);
        lightUBO.writeElement<glm::vec4>(&pointLights[i]->diffuse);
        lightUBO.writeElement<glm::vec4>(&pointLights[i]->specular);
        lightUBO.writeElement<float>(&pointLights[i]->k0);
        lightUBO.writeElement<float>(&pointLights[i]->k1);
        lightUBO.writeElement<float>(&pointLights[i]->k2);
        lightUBO.writeElement<float>(&pointLights[i]->farPlane);
    }
    lightUBO.advanceArray(MAX_POINT_LIGHTS - i); // advance to finish array

    // spot lights
    noSpotLights = std::min<unsigned int>(spotLights.size(), MAX_SPOT_LIGHTS);
    lightUBO.writeElement<unsigned int>(&noSpotLights);
    for (i = 0; i < noSpotLights; i++) {
        lightUBO.writeElement<glm::vec3>(&spotLights[i]->position);
        lightUBO.writeElement<glm::vec3>(&spotLights[i]->direction);
        lightUBO.writeElement<float>(&spotLights[i]->cutOff);
        lightUBO.writeElement<float>(&spotLights[i]->outerCutOff);
        lightUBO.writeElement<glm::vec4>(&spotLights[i]->ambient);
        lightUBO.writeElement<glm::vec4>(&spotLights[i]->diffuse);
        lightUBO.writeElement<glm::vec4>(&spotLights[i]->specular);
        lightUBO.writeElement<float>(&spotLights[i]->k0);
        lightUBO.writeElement<float>(&spotLights[i]->k1);
        lightUBO.writeElement<float>(&spotLights[i]->k2);
        lightUBO.writeElement<float>(&spotLights[i]->nearPlane);
        lightUBO.writeElement<float>(&spotLights[i]->farPlane);
        lightUBO.writeArrayContainer<glm::mat4, glm::vec4>(&spotLights[i]->lightSpaceMatrix, 4);
    }

    lightUBO.clear();
}

/*
    main loop methods
*/

// process input
void Scene::processInput(float dt) {
    if (activeCamera != -1 && activeCamera < cameras.size()) {
        // active camera exists

        //defines default values for x and y mouse movement
        static double dx = scrWidth/2; 
        static double dy = scrHeight/2;

        // set camera direction
        dx = Mouse::getDX();
        dy = Mouse::getDY();
        if (dx != 0 || dy != 0) {
            cameras[activeCamera]->updateCameraDirection(dx, dy);
        }

        // set camera zoom
        double scrollDy = Mouse::getScrollDY();
        if (scrollDy != 0) {
            cameras[activeCamera]->updateCameraZoom(scrollDy);
        }

        const Uint8 *state = SDL_GetKeyboardState(NULL);

        // set camera pos
        if (Keyboard::key(SDLK_w)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::FORWARD, dt);
        }
        if (Keyboard::key(SDLK_s)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::BACKWARD, dt);
        }
        if (Keyboard::key(SDLK_d)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::RIGHT, dt);
        }
        if (Keyboard::key(SDLK_a)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::LEFT, dt);
        }
        if (Keyboard::key(SDLK_SPACE)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::UP, dt);
        }
        if (Keyboard::key(SDLK_LCTRL)) {
            cameras[activeCamera]->updateCameraPos(CameraDirection::DOWN, dt);
        }

        // set matrices
        view = cameras[activeCamera]->getViewMatrix();
        projection = glm::perspective(
            glm::radians(cameras[activeCamera]->getZoom()),	    // FOV
            (float)scrWidth / (float)scrHeight,					// aspect ratio
            0.1f, 100.0f										// near and far bounds
        );
        textProjection = glm::ortho(0.0f, (float)scrWidth, 0.0f, (float)scrHeight);

        // set pos
        cameraPos = cameras[activeCamera]->cameraPos;
    }
}



// update screen before each frame
void Scene::update() {
    // set background color
    glClearColor(bg[0], bg[1], bg[2], bg[4]);
    // clear occupied bits
    defaultFBO.clear();
}


// update screen after frame
void Scene::newFrame(Box &box) {
    box.positions.clear();
    box.sizes.clear();

    // process pending objects
    octree->processPending();
    octree->update(box);

    // send new frame to window
    SDL_GL_SwapWindow(window);
    //update inputs in scene
    SDL_PollEvent(&event);
    Keyboard::keyCallback(event);

}

// set uniform shader varaibles (lighting, etc)
void Scene::renderShader(Shader shader, bool applyLighting) {
    // activate shader
    shader.activate();

    // set camera values
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.set3Float("viewPos", cameraPos);

    // lighting
    if (applyLighting) {
        unsigned int textureIdx = 31;

        // directional light
        dirLight->render(shader, textureIdx--);

        // point lights
        unsigned int noLights = pointLights.size();
        unsigned int noActiveLights = 0;
        for (unsigned int i = 0; i < noLights; i++) {
            if (States::isIndexActive(&activePointLights, i)) {
                // i'th light is active
                pointLights[i]->render(shader, noActiveLights, textureIdx--);
                noActiveLights++;
            }
        }
        shader.setInt("noPointLights", noActiveLights);

        // spot lights
        noLights = spotLights.size();
        noActiveLights = 0;
        for (unsigned int i = 0; i < noLights; i++) {
            if (States::isIndexActive(&activeSpotLights, i)) {
                // i'th spot light active
                spotLights[i]->render(shader, noActiveLights, textureIdx--);
                noActiveLights++;
            }
        }
        shader.setInt("noSpotLights", noActiveLights);
    }
}
// set uniform shader variables for directional light render
void Scene::renderDirLightShader(Shader shader) {
    shader.activate();
    shader.setMat4("lightSpaceMatrix", dirLight->lightSpaceMatrix);
}

// set uniform shader variables for point light render
void Scene::renderPointLightShader(Shader shader, unsigned int idx) {
    shader.activate();

    // light space matrices
    for (unsigned int i = 0; i < 6; i++) {
        shader.setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", pointLights[idx]->lightSpaceMatrices[i]);
    }

    // light position
    shader.set3Float("lightPos", pointLights[idx]->position);

    // far plane
    shader.setFloat("farPlane", pointLights[idx]->farPlane);
}

// set uniform shader variables for spot light render
void Scene::renderSpotLightShader(Shader shader, unsigned int idx) {
    shader.activate();

    // light space matrix
    shader.setMat4("lightSpaceMatrix", spotLights[idx]->lightSpaceMatrix);

    // light position
    shader.set3Float("lightPos", spotLights[idx]->position);

    // far plane
    shader.setFloat("farPlane", spotLights[idx]->farPlane);
}

// render specified model's instances
void Scene::renderInstances(std::string modelId, Shader shader, float dt) {
    void* val = avl_get(models, (void*)modelId.c_str());
    if (val) {
        // render each mesh in specified model
        shader.activate();
        ((Model*)val)->render(shader, dt, this);
    }
}

// render text
void Scene::renderText(std::string font, Shader shader, std::string text, float x, float y, glm::vec2 scale, glm::vec3 color) {
    void* val = avl_get(fonts, (void*)font.c_str());
    if (val) {
        shader.activate();
        shader.setMat4("projection", textProjection);

        ((TextRenderer*)val)->render(shader, text, x, y, scale, color);
    }
}

/*
    cleanup method
*/

// called after main loop
void Scene::cleanup() {
    // clean up instances
    instances.cleanup();

    // clean all models
    avl_postorderTraverse(models, [](avl* node) -> void {
        ((Model*)node->val)->cleanup();
    });
    avl_free(models);

    // cleanup fonts
    avl_postorderTraverse(fonts, [](avl* node) -> void {
        ((TextRenderer*)node->val)->cleanup();
    });
    avl_free(fonts);

    // destroy octree
    octree->destroy();

    // quit SDL
    //glfwTerminate();
    SDL_DestroyWindow(window);
    SDL_Quit(); 
}

/*
    accessors
*/

// determine if window should close
bool Scene::shouldClose() {
    return (event.type == SDL_QUIT);
}

// get current active camera in scene
Camera* Scene::getActiveCamera() {
    // if activate camera exists, return the one at that index
    return (activeCamera >= 0 && activeCamera < cameras.size()) ? cameras[activeCamera] : nullptr;
}

/*
    modifiers
*/

// set if the window should close
void Scene::setShouldClose(bool shouldClose) {
    if(shouldClose) 
        event.type = SDL_QUIT;
}

// set window background color
void Scene::setWindowColor(float r, float g, float b, float a) {
    // set rgba vals of the background color
    bg[0] = r;
    bg[1] = g;
    bg[2] = b;
    bg[3] = a;
}

/*
    Model/instance methods
*/

// register model into model trie
void Scene::registerModel(Model* model) {
    models = avl_insert(models, (void*)model->id.c_str(), model);
}

// generate instance of specified model with physical parameters
RigidBody* Scene::generateInstance(std::string modelId, glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot) {
    // generate new rigid body
    void* val = avl_get(models, (void*)modelId.c_str());
    if (val) {
        Model* model = (Model*)val;
        RigidBody* rb = model->generateInstance(size, mass, pos, rot);
        if (rb) {
            // successfully generated, set new and unique id for instance
            std::string id = generateId();
            rb->instanceId = id;
            // insert into trie
            instances.insert(rb->instanceId, rb);
            // insert into pending queue
            octree->addToPending(rb, model);
            return rb;
        }
    }
    return nullptr;
}

// initialize model instances
void Scene::initInstances() {
    // initialize all instances for each model
    avl_inorderTraverse(models, [](avl* node) -> void {
        ((Model*)node->val)->initInstances();
    });
}

// load model data
void Scene::loadModels() {
    // initialize each model
    avl_inorderTraverse(models, [](avl* node) -> void {
        ((Model*)node->val)->init();
    });
}

// delete instance
void Scene::removeInstance(std::string instanceId) {
    RigidBody* instance = instances[instanceId];
    // get instance's model
    std::string targetModel = instance->modelId;
    Model* model = (Model*)avl_get(models, (void*)targetModel.c_str());

    // delete instance from model
    model->removeInstance(instanceId);

    // remove from tree
    instances[instanceId] = NULL;
    instances.erase(instanceId);
    free(instance);
}

// mark instance for deletion
void Scene::markForDeletion(std::string instanceId) {
    RigidBody* instance = instances[instanceId];

    // activate kill switch
    States::activate(&instance->state, INSTANCE_DEAD);
    // push to list
    instancesToDelete.push_back(instance);
}

// clear all instances marked for deletion
void Scene::clearDeadInstances() {
    for (RigidBody* rb : instancesToDelete) {
        removeInstance(rb->instanceId);
    }
    instancesToDelete.clear();
}

// generate next instance id
std::string Scene::generateId() {
    for (int i = currentId.length() - 1; i >= 0; i--) {
        if ((int)currentId[i] != (int)'z') {
            // increment then break
            currentId[i] = (char)(((int)currentId[i]) + 1);
            break;
        }
        else {
            currentId[i] = 'a';
        }
    }
    return currentId;
}