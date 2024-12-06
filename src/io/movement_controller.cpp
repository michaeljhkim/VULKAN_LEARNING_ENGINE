#include "movement_controller.hpp"
#include "../physics/environment.hpp"

/*
fov/zoom stuff we can keep here because no where else needs these values... FOR NOW (add to header fiile later if needed elsewhere)
*/
float camera_fov = 75.0f;
float max_zoom_out = 100.0f;

/*
    constructor
*/

// default and initialize with position
MovementController::MovementController(glm::vec3 position)
    : cameraPos(position),
    yaw(0.0f),
    pitch(0.0f),
    speed(2.5f),
    sensitivity(0.09f),
    zoom(camera_fov),
    cameraFront(glm::vec3(1.0f, 0.0f, 0.0f)) {
    updateCameraVectors();
}

/*
    modifiers
*/

// change camera direction (mouse movement)
void MovementController::updateCameraDirection(double dx, double dy) {
    yaw += dx * sensitivity;
    pitch += dy * sensitivity;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    updateCameraVectors();
}

// change camera position in certain direction (keyboard)
void MovementController::updateCameraPos(CameraDirection direction, double dt) {
    float velocity = (float)dt * speed;

    switch (direction) {
    case CameraDirection::FORWARD:
        cameraPos += cameraFront * velocity;
        break;
    case CameraDirection::BACKWARD:
        cameraPos -= cameraFront * velocity;
        break;
    case CameraDirection::RIGHT:
        cameraPos += cameraRight * velocity;
        break;
    case CameraDirection::LEFT:
        cameraPos -= cameraRight * velocity;
        break;
    case CameraDirection::UP:
        cameraPos += cameraUp * velocity;
        break;
    case CameraDirection::DOWN:
        cameraPos -= cameraUp * velocity;
        break;
    }
}

// change camera zoom (scroll wheel)
void MovementController::updateCameraZoom(double dy) {
    float new_zoom = zoom - dy*2;

    //stop being able to scroll back if new_zoom is the same as default fov size
    if(new_zoom >= camera_fov) {
        zoom = camera_fov;
    }
    else if (new_zoom >= 1.0f && new_zoom < camera_fov) {
        zoom = new_zoom;
    }
    else {
        zoom = 1.0f;
    } 
}


/*
    accessors
*/

// get view matrix for camera
glm::mat4 MovementController::getViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// get zoom value for camera
float MovementController::getZoom() {
    return zoom;
}

/*
    private modifier
*/

// change camera directional vectors based on movement
void MovementController::updateCameraVectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    cameraRight = glm::normalize(glm::cross(cameraFront, Environment::worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}



void MovementController::moveInPlaneXZ(float dt, GameObject& gameObject) {
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
	if (Keyboard::key(Keyboard::key_mappings.moveForward)) moveDir += forwardDir;
	if (Keyboard::key(Keyboard::key_mappings.moveBackward)) moveDir -= forwardDir;
	if (Keyboard::key(Keyboard::key_mappings.moveRight)) moveDir += rightDir;
	if (Keyboard::key(Keyboard::key_mappings.moveLeft)) moveDir -= rightDir;
	if (Keyboard::key(Keyboard::key_mappings.moveUp)) moveDir += upDir;
	if (Keyboard::key(Keyboard::key_mappings.moveDown)) moveDir -= upDir;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	}
}