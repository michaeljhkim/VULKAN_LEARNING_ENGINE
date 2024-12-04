#include "keyboard_movement_controller.hpp"

// std
#include <limits>

//namespace lve {

/*
	Static variables
*/
// key state array (true for down, false for up)
//bool Keyboard::keys[GLFW_KEY_LAST] = { 0 };
// Get the number of keys (usually at least 512)
constexpr int KEY_COUNT = 512; // Maximum number of key states

// Global variables to store keyboard states
bool KeyboardMovementController::keys[KEY_COUNT] = { 0 };
bool KeyboardMovementController::keysChanged[KEY_COUNT] = { 0 };

/*
	Static callback
*/
void KeyboardMovementController::keyCallback(SDL_Event event) {
	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		SDL_Scancode keyPressed = event.key.keysym.scancode;
		bool isDown = (event.type == SDL_KEYDOWN);

		// Update key state and change status
		keysChanged[keyPressed] = (keys[keyPressed] != isDown) && !event.key.repeat;
		keys[keyPressed] = isDown;
	}
}

/*
	Static accessors
*/
bool KeyboardMovementController::key(SDL_Scancode input_key) {
	return keys[static_cast<int>(input_key)];
}

bool KeyboardMovementController::keyChanged(SDL_Scancode input_key) {
	int keyInt = static_cast<int>(input_key);
	bool changed = keysChanged[keyInt];
	keysChanged[keyInt] = false; // Reset change status
	return changed;
}

bool KeyboardMovementController::keyWentDown(SDL_Scancode input_key) {
	return key(input_key) && keyChanged(input_key);
}

bool KeyboardMovementController::keyWentUp(SDL_Scancode input_key) {
	return !key(input_key) && keyChanged(input_key);
}



void KeyboardMovementController::moveInPlaneXZ(float dt, GameObject& gameObject) {
	glm::vec3 rotate{0};
	//if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
	//if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
	//if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
	//if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
	}

	// limit pitch values between about +/- 85ish degrees
	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.transform.rotation.y;
	const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
	const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
	const glm::vec3 upDir{0.0f, -1.0f, 0.0f};

	glm::vec3 moveDir{0.0f};
	if (key(key_mappings.moveForward)) moveDir += forwardDir;
	if (key(key_mappings.moveBackward)) moveDir -= forwardDir;
	if (key(key_mappings.moveRight)) moveDir += rightDir;
	if (key(key_mappings.moveLeft)) moveDir -= rightDir;
	if (key(key_mappings.moveUp)) moveDir += upDir;
	if (key(key_mappings.moveDown)) moveDir -= upDir;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	}
}

//}	// namespace lve