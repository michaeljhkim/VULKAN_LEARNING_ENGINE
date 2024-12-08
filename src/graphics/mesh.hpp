#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "vulkan_buffer.hpp"
#include <vulkan/vulkan.hpp>

#include <vector>
#include <glm/glm.hpp>

#include "shader_pipeline.hpp"
#include "rendering/texture.hpp"
#include "rendering/material.hpp"

#include "memory/vertexmemory.hpp"
#include "models/box.hpp"
#include "../algorithms/bounds.hpp"
#include "../physics/collisionmesh.hpp"

/*
    structure storing values for each vertex
*/

struct Vertex {
    /*
        vertex values
    */
    // position
    glm::vec3 position{};

    // color
    glm::vec3 color{};

    // normal vector
    glm::vec3 normal{};

    // texture coordinate (also known as UV or uv)
    glm::vec2 texCoord{};

    // tangent vector
    glm::vec3 tangent;
    // generate list of vertices
    static std::vector<Vertex> genList(float* vertices, int numVertices);
    // calculate tangent vectors for each face
    static void calcTanVectors(std::vector<Vertex>& list, std::vector<unsigned int>& indices);

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
            // Binding for vertex data
            {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX},

            // Binding for instance data
            {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},

            // Binding for normalized instance data (e.g., orientation)
            {2, sizeof(glm::mat3), VK_VERTEX_INPUT_RATE_INSTANCE}
        };
        return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
            // Per-vertex attributes
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)},

            // Per-instance attributes (model matrix columns)
            //{4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 * sizeof(glm::vec4)}, // First column
            //{5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 1 * sizeof(glm::vec4)}, // Second column
            {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
            {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT,     sizeof(glm::vec4)},
            {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 2 * sizeof(glm::vec4)}, // Third column
            {7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 3 * sizeof(glm::vec4)}, // Fourth column

            // Per-instance attributes (normalized matrix columns)
            //{ 8, 2, VK_FORMAT_R32G32B32_SFLOAT, 0 * sizeof(glm::vec3)},  // First column of glm::mat3
            //{ 9, 2, VK_FORMAT_R32G32B32_SFLOAT, 1 * sizeof(glm::vec3)},  // Second column of glm::mat3
            { 8, 2, VK_FORMAT_R32G32B32_SFLOAT, 0},
            { 9, 2, VK_FORMAT_R32G32B32_SFLOAT,     sizeof(glm::vec3)}, 
            {10, 2, VK_FORMAT_R32G32B32_SFLOAT, 2 * sizeof(glm::vec3)}   // Third column of glm::mat3
        };

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const {
        return (position == other.position) && (color == other.color) && (normal == other.normal) && (texCoord == other.texCoord);
    }
};

/*
    class representing Mesh
*/

class Mesh {
public:
    // Bounding region for mesh
    BoundingRegion br;
    // pointer to the attached collision mesh
    std::unique_ptr<CollisionMesh> collision;

    // list of vertices
    std::vector<Vertex> vertices;
    // list of indices
    std::vector<unsigned int> indices;

    // vertex array object pointing to all data for the mesh
    ArrayObject VAO;

    // texture list
    std::vector<Texture> textures;
    // material diffuse value
    aiColor4D diffuse;
    // material specular value
    aiColor4D specular;

    /*
        constructors
    */

    // default
    Mesh();

    // intialize with a bounding region
    Mesh(BoundingRegion br);

    // initialize as textured object
    Mesh(BoundingRegion br, std::vector<Texture> textures);

    // initialize as material object
    Mesh(BoundingRegion br, aiColor4D diff, aiColor4D spec);

    // initialize with a material
    Mesh(BoundingRegion br, Material m);

    // load vertex and index data
    void loadData(std::vector<Vertex> vertices, std::vector<unsigned int> indices, bool pad = false);

    // setup collision mesh
    void loadCollisionMesh(unsigned int numPoints, float* coordinates, unsigned int numFaces, unsigned int* indices);

    // setup textures
    void setupTextures(std::vector<Texture> textures);

    // setup material colors
    void setupColors(aiColor4D diff, aiColor4D spec);

    // set material structure
    void setupMaterial(Material mat);

    // render number of instances using shader
    void render(ShaderPipline& shader_pipeline, unsigned int noInstances);

    // free up memory
    void cleanup();

private:
    // true if has only materials
    bool TexExists;
    void createMaterialBuffers();

    // setup data with buffers
    //void setup();
};
