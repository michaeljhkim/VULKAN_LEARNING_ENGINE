#pragma once

#include <functional>
#include <glm/gtc/matrix_transform.hpp>

//namespace lve {

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
	seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hashCombine(seed, rest), ...);
};






struct TransformComponent {
	glm::vec3 translation{};
	glm::vec3 scale{1.f, 1.f, 1.f};
	glm::vec3 rotation{};

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4();
	glm::mat3 normalMatrix();
};

glm::mat4 TransformComponent::mat4() {
    // Precompute trigonometric values
    const float cX = glm::cos(rotation.x), sX = glm::sin(rotation.x);
    const float cY = glm::cos(rotation.y), sY = glm::sin(rotation.y);
    const float cZ = glm::cos(rotation.z), sZ = glm::sin(rotation.z);

    // Compute common terms
    const float cYcZ = cY * cZ, cYsZ = cY * sZ;
    const float sYsZ = sY * sZ, sYcZ = sY * cZ;
    const float sXcZ = sX * cZ, sXsZ = sX * sZ;
    const float cXsY = cX * sY, cXcY = cX * cY;

    // Construct the transformation matrix
    return glm::mat4{
        // First column
        {scale.x * (cYcZ + sYsZ * sX), scale.x * (cX * sZ), scale.x * (sYcZ * sX - cYsZ), 0.0f},
        // Second column
        {scale.y * (sYsZ * cX - cYcZ * sX), scale.y * (cX * cZ), scale.y * (cYsZ * sX + sYcZ), 0.0f},
        // Third column
        {scale.z * (cXsY), scale.z * (-sX), scale.z * (cXcY), 0.0f},
        // Fourth column (translation)
        {translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat3 TransformComponent::normalMatrix() {
    // Precompute trigonometric values
    const float cX = glm::cos(rotation.x), sX = glm::sin(rotation.x);
    const float cY = glm::cos(rotation.y), sY = glm::sin(rotation.y);
    const float cZ = glm::cos(rotation.z), sZ = glm::sin(rotation.z);

    // Inverse scale factors
    const glm::vec3 invScale = 1.0f / scale;

    // Precompute common terms
    const float cYcZ = cY * cZ, cYsZ = cY * sZ;
    const float sYsZ = sY * sZ, sYcZ = sY * cZ;
    const float sXcZ = sX * cZ, sXsZ = sX * sZ;
    const float cXsY = cX * sY, cXcY = cX * cY;

    // Return the normal matrix
    return glm::mat3{
        // First row
        {invScale.x * (cYcZ + sYsZ * sX), invScale.x * (cX * sZ), invScale.x * (sYcZ * sX - cYsZ)},
        // Second row
        {invScale.y * (sYsZ * cX - cYcZ * sX), invScale.y * (cX * cZ), invScale.y * (cYsZ * sX + sYcZ)},
        // Third row
        {invScale.z * (cXsY), invScale.z * (-sX), invScale.z * (cXcY)},
    };
}

//}  // namespace lve
