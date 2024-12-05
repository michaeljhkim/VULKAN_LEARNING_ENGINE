#include "shader.hpp"

#include <stdio.h>
#include <fstream>

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>


/*
    constructors
*/

// default
Shader::Shader() {}

// initialize with paths to vertex and fragment shaders
Shader::Shader(bool includeDefaultHeader, const char* vertexShaderPath, const char* fragShaderPath, const char* geoShaderPath) {
    generate(includeDefaultHeader, vertexShaderPath, fragShaderPath, geoShaderPath);
}

/*
    process functions
*/

void compileAndAttach(GLuint id, bool includeDefaultHeader, const char* path, GLuint type) {
    if (!path) {
        return;
    }

    GLuint shader = Shader::compileShader(includeDefaultHeader, path, type);
    glAttachShader(id, shader);
    glDeleteShader(shader);
}

// generate using vertex and frag shaders
void Shader::generate(bool includeDefaultHeader, const char* vertexShaderPath, const char* fragShaderPath, const char* geoShaderPath) {
    id = glCreateProgram();

    // compile and attach shaders
    compileAndAttach(id, includeDefaultHeader, vertexShaderPath, GL_VERTEX_SHADER);
    compileAndAttach(id, includeDefaultHeader, fragShaderPath, GL_FRAGMENT_SHADER);
    compileAndAttach(id, includeDefaultHeader, geoShaderPath, GL_GEOMETRY_SHADER);
    glLinkProgram(id);

    // linking errors
    int success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char* infoLog = (char*)malloc(512);
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "Linking error:" << std::endl << infoLog << std::endl;
    }
}

// activate shader
void Shader::activate() {
    glUseProgram(id);
}

// cleanup
void Shader::cleanup() {
    glDeleteProgram(id);
}

/*
    set uniform variables
*/

void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set3Float(const std::string& name, float v1, float v2, float v3) {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void Shader::set3Float(const std::string& name, glm::vec3 v) {
    glUniform3f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z);
}

void Shader::set4Float(const std::string& name, float v1, float v2, float v3, float v4) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v1, v2, v3, v4);
}

void Shader::set4Float(const std::string& name, aiColor4D color) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), color.r, color.g, color.b, color.a);
}

void Shader::set4Float(const std::string& name, glm::vec4 v) {
    glUniform4f(glGetUniformLocation(id, name.c_str()), v.x, v.y, v.z, v.w);
}

void Shader::setMat3(const std::string& name, glm::mat3 val) {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::setMat4(const std::string& name, glm::mat4 val) {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}


/*
void initializeGlslang() {
    glslang::InitializeProcess();
}

void finalizeGlslang() {
    glslang::FinalizeProcess();
}
*/

/*
    static
*/

// compile shader program
GLuint Shader::compileShader(bool includeDefaultHeader, const char* filePath, GLuint type) {
    // create shader from file
    GLuint ret = glCreateShader(type);
    GLchar* shader = loadShaderSrc(includeDefaultHeader, filePath);
    glShaderSource(ret, 1, &shader, NULL);
    glCompileShader(ret);
    free(shader);

    // catch compilation error
    int success;
    glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
    if (!success) {
        char* infoLog = (char*)malloc(512);
        glGetShaderInfoLog(ret, 512, NULL, infoLog);
        std::cout << "Error with shader comp." << filePath << ":" << std::endl << infoLog << std::endl;
    }

    return ret;
}



// stream containing default headers
std::stringstream Shader::defaultHeaders;

// load into default header
void Shader::loadIntoDefault(const char* filepath) {
    // Load shader source as a string (no need for manual memory management)
    std::string fileContents = Shader::loadShaderSrc(false, filepath);
    // Append file contents directly to the default headers
    Shader::defaultHeaders << fileContents;
}

// clear default header (after shader compilation)
void Shader::clearDefault() {
    Shader::defaultHeaders.clear();
}

// load string from file
char* Shader::loadShaderSrc(bool includeDefaultHeader, const char* filePath) {
    // Construct full file path
    std::string fullPath = Shader::defaultDirectory + '/' + filePath;

    // Open the file in binary mode
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Could not open " << filePath << std::endl;
        return nullptr;
    }

    // Get file size
    std::streamsize len = file.tellg();
    file.seekg(0, std::ios::beg);

    // Determine total size with or without the default header
    size_t totalSize = includeDefaultHeader ? (Shader::defaultHeaders.str().size() + len) : len;

    // Create a buffer for the shader source (using vector for automatic memory management)
    std::vector<char> ret(totalSize + 1);  // Extra byte for null terminator

    // Copy default header if necessary
    if (includeDefaultHeader) {
        std::copy(Shader::defaultHeaders.str().begin(), Shader::defaultHeaders.str().end(), ret.begin());
    }

    // Read the shader source into the buffer
    file.read(ret.data() + (includeDefaultHeader ? Shader::defaultHeaders.str().size() : 0), len);

    // Null-terminate the string
    ret[totalSize] = '\0';

    // Extract just the file name from the full path using std::filesystem
    //std::string fileName = std::filesystem::path(fullPath).filename().string();

    // Return a raw pointer to the data
    return ret.data();
}




// Helper function to read a file into a string
std::string Shader::ReadFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    size_t fileSize = (size_t)file.tellg();
    std::string buffer(fileSize, '\0');
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

void Shader::saveSPIRVToFile(const std::string& filename, const std::vector<uint32_t>& spirvCode) {
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file.write(reinterpret_cast<const char*>(spirvCode.data()), spirvCode.size() * sizeof(uint32_t));
    file.close();
}




// Compile GLSL to SPIR-V
std::vector<uint32_t> Shader::CompileGLSLToSPIRV(const std::string& glslSource, EShLanguage shaderType) {
    glslang::TShader shader(shaderType);
    const char* sourceCStr = glslSource.c_str();
    shader.setStrings(&sourceCStr, 1);
    
    TBuiltInResource defaultResource = InitResources();

    // Parse GLSL
    if (!shader.parse(&defaultResource, 100, false, EShMsgDefault)) {
        throw std::runtime_error("GLSL Parsing Failed: " + std::string(shader.getInfoLog()));
    }

    // Link into a program
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault)) {
        throw std::runtime_error("GLSL Linking Failed: " + std::string(program.getInfoLog()));
    }

    // Convert to SPIR-V
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv);

    return spirv;
}