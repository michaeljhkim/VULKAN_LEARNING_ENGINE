#include "camera.h"
#include "../physics/environment.h"

/*
fov/zoom stuff we can keep here because no where else needs these values... FOR NOW (add to header fiile later if needed elsewhere)
*/
float camera_fov = 75.0f;
float max_zoom_out = 100.0f;

/*
    constructor
*/

// default and initialize with position
Camera::Camera(glm::vec3 position)
    : cameraPos(position),
    yaw(0.0f),
    pitch(0.0f),
    speed(2.5f),
    sensitivity(0.09f),
    zoom(camera_fov),
    cameraFront(glm::vec3(1.0f, 0.0f, 0.0f))
{
    updateCameraVectors();
}

/*
    modifiers
*/

// change camera direction (mouse movement)
void Camera::updateCameraDirection(double dx, double dy) {
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
void Camera::updateCameraPos(CameraDirection direction, double dt) {
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
void Camera::updateCameraZoom(double dy) {
    float new_zoom = zoom - dy*2;

    //stop being able to scroll back if new_zoom is the same as default fov size
    if(new_zoom >= camera_fov) {
        zoom = camera_fov;
    }
    else if (new_zoom >= 1.0f && new_zoom < camera_fov) {
        zoom = new_zoom;
    }
    //else if (new_zoom < 1.0f)
    else { // < 1.0f
        zoom = 1.0f;
    } 
}


/*
    accessors
*/

// get view matrix for camera
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// get zoom value for camera
float Camera::getZoom() {
    return zoom;
}

/*
    private modifier
*/

// change camera directional vectors based on movement
void Camera::updateCameraVectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    cameraRight = glm::normalize(glm::cross(cameraFront, Environment::worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}