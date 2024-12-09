#include "mesh.hpp"
#include <iostream>


// generate list of vertices
std::vector<Vertex> Vertex::genList(float* vertices, int numVertices) {
    std::vector<Vertex> ret(numVertices);
    int stride = 8;

    for (int i = 0; i < numVertices; i++) {
        ret[i].position = glm::vec3(
            vertices[i * stride + 0],
            vertices[i * stride + 1],
            vertices[i * stride + 2]
        );

        ret[i].normal = glm::vec3(
            vertices[i * stride + 3],
            vertices[i * stride + 4],
            vertices[i * stride + 5]
        );

        ret[i].texCoord = glm::vec2(
            vertices[i * stride + 6],
            vertices[i * stride + 7]
        );
    }

    return ret;
}

void averageVectors(glm::vec3& baseVec, glm::vec3 addition, unsigned char existingContributions) {
    if (!existingContributions) {
        baseVec = addition;
    }
    else {
        float f = 1 / ((float)existingContributions + 1);
        baseVec *= (float)(existingContributions)*f;
        baseVec += addition * f;
    }
}

// calculate tangent vectors for each face
void Vertex::calcTanVectors(std::vector<Vertex>& list, std::vector<unsigned int>& indices) {
    unsigned char* counts = (unsigned char*)malloc(list.size() * sizeof(unsigned char));
    for (unsigned int i = 0, len = list.size(); i < len; i++) {
        counts[i] = 0;
    }

    // iterate through indices and calculate vectors for each face
    for (unsigned int i = 0, len = indices.size(); i < len; i += 3) {
        // 3 vertices corresponding to the face
        Vertex v1 = list[indices[i + 0]];
        Vertex v2 = list[indices[i + 1]];
        Vertex v3 = list[indices[i + 2]];

        // calculate edges
        glm::vec3 edge1 = v2.position - v1.position;
        glm::vec3 edge2 = v3.position - v1.position;

        // calculate dUVs
        glm::vec2 deltaUV1 = v2.texCoord - v1.texCoord;
        glm::vec2 deltaUV2 = v3.texCoord - v1.texCoord;

        // use inverse of the UV matrix to determine tangent
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

        glm::vec3 tangent = {
            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
        };

        // average in the new tangent vector
        averageVectors(list[indices[i + 0]].tangent, tangent, counts[indices[i + 0]]++);
        averageVectors(list[indices[i + 1]].tangent, tangent, counts[indices[i + 1]]++);
        averageVectors(list[indices[i + 2]].tangent, tangent, counts[indices[i + 2]]++);
    }
}

/*
    constructors
*/

// default
Mesh::Mesh() {
    this->collision = std::make_unique<CollisionMesh>(NULL);
}

// Constructor with bounding region
Mesh::Mesh(BoundingRegion br) {
    this->br = br;
    this->collision = std::make_unique<CollisionMesh>(NULL);
}

// initialize as textured object
Mesh::Mesh(BoundingRegion br, std::vector<Texture> textures) : Mesh(br) {
    setupTextures(textures);
}

// initialize as material object
Mesh::Mesh(BoundingRegion br, aiColor4D diff, aiColor4D spec) : Mesh(br) {
    setupColors(diff, spec);
}

// initialize with a material
Mesh::Mesh(BoundingRegion br, Material m) : Mesh(br) {
    setupMaterial(m);
}




// load vertex and index data
void Mesh::loadData(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices, bool pad) {
    this->vertices = _vertices;
    this->indices = _indices;
}

// setup collision mesh
void Mesh::loadCollisionMesh(unsigned int numPoints, float* coordinates, unsigned int numFaces, unsigned int* indices) {
    this->collision = std::make_unique<CollisionMesh>(numPoints, coordinates, numFaces, indices);
    this->br = this->collision->br;
}

// setup textures
void Mesh::setupTextures(std::vector<Texture> textures) {
    this->noTextures = false;
    this->textures.insert(this->textures.end(), textures.begin(), textures.end());
}

// setup material colors
void Mesh::setupColors(aiColor4D diff, aiColor4D spec) {
    this->noTextures = true;
    this->diffuse = diff;
    this->specular = spec;
}

// set material structure
void Mesh::setupMaterial(Material mat) {
    this->noTextures = true;
    this->diffuse = { mat.diffuse.r, mat.diffuse.g, mat.diffuse.b, 1.0f };
    this->specular = { mat.specular.r, mat.specular.g, mat.specular.b, 1.0f };
}

// render number of instances using shader
void Mesh::render(ShaderPipline& shader_pipeline, unsigned int numInstances) {
    shader_pipeline.setBool("noNormalMap", true);

    if (noTextures) {
        // materials
        shader_pipeline.set4Float("material.diffuse", diffuse);
        shader_pipeline.set4Float("material.specular", specular);
        shader_pipeline.setBool("noTextures", true);
    }
    else {
        // textures
        unsigned int diffuseIdx = 0;
        unsigned int normalIdx = 0;
        unsigned int specularIdx = 0;

        for (unsigned int i = 0; i < textures.size(); i++) {
            // retrieve texture info
            std::string name;
            switch (textures[i].type) {
            case aiTextureType_DIFFUSE:
                name = "diffuse" + std::to_string(diffuseIdx++);
                break;
            case aiTextureType_NORMALS:
                name = "normal" + std::to_string(normalIdx++);
                shader_pipeline.setBool("noNormalMap", false);
                break;
            case aiTextureType_SPECULAR:
                name = "specular" + std::to_string(specularIdx++);
                break;
            default:
                name = textures[i].name;
                break;
            }

            // set the shader value and bind texture
            shader_pipeline.setInt(name, i);
            textures[i].bind();
        }

        shader_pipeline.setBool("noTextures", false);
    }
}



// free up memory
void Mesh::cleanup() {
    VAO.cleanup();

    for (Texture t : textures) {
        t.cleanup();
    }
}





// setup data with buffers
//void Mesh::setup() {}