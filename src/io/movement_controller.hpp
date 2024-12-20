#include <iostream>

#include "keyboard.hpp"
#include "mouse.hpp"

#include "../graphics/game_object.hpp"
#include "../graphics/vulkan_window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


/*
    enum to represent directions for movement
*/

enum class CameraDirection {
    NONE = 0,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/*
    camera class to help display from POV of camera
*/

class MovementController {
public:
    /*
        camera values
    */

    // position
    glm::vec3 cameraPos;

    // camera directional values
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;

    // camera rotational values
    float yaw; // x-axis
    float pitch; // y-axis

    // camera movement values
    float speed;
    float sensitivity;
    float zoom;

    /*
        constructor
    */

    // default and initialize with position
    MovementController(glm::vec3 position = glm::vec3(0.0f));

    /*
        modifiers
    */

    // change camera direction (mouse movement)
    void updateCameraDirection(double dx, double dy);

    // change camera position in certain direction (keyboard)
    void updateCameraPos(CameraDirection direction, double dt);

    // change camera zoom (scroll wheel)
    void updateCameraZoom(double dy);

    /*
        accessors
    */

    // get view matrix for camera
    glm::mat4 getViewMatrix();

    // get zoom value for camera
    float getZoom();

    void moveInPlaneXZ(float dt, GameObject& gameObject);
	float moveSpeed{3.f};
	float lookSpeed{1.5f};

private:
    /*
        private modifier
    */

    // change camera directional vectors based on movement
    void updateCameraVectors();
};
