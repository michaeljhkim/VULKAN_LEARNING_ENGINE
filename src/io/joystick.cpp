#include "joystick.h"

/*
    constructor
*/

/*
// generate an instance for joystick with id i
Joystick::Joystick(int i) {
    id = getId(i);

    update();
}
*/

/*
// update the joystick's states
void Joystick::update() {
    present = glfwJoystickPresent(id);

    if (present) {
        name = glfwGetJoystickName(id);
        axes = glfwGetJoystickAxes(id, &axesCount);
        buttons = glfwGetJoystickButtons(id, &buttonCount);
    }
}
*/
// Constructor for initializing joystick with id
Joystick::Joystick(int i) : sdl_joystick(nullptr) {
    sdl_joystick = SDL_JoystickOpen(i);
    if (sdl_joystick) {
        id = i;
        name = SDL_JoystickName(sdl_joystick);
        axesCount = SDL_JoystickNumAxes(sdl_joystick);
        buttonCount = SDL_JoystickNumButtons(sdl_joystick);

        // Initialize axes and button arrays
        axes.resize(axesCount, 0.0f);
        buttons.resize(buttonCount, 0.0f);
    }
}


// Update the joystick's states
void Joystick::update() {
    present = SDL_JoystickGetAttached(sdl_joystick);
    if (present) {
        for (int j = 0; j < axesCount; ++j) {
            axes[j] = SDL_JoystickGetAxis(sdl_joystick, j) / 32767.0f; // Normalize axis values
        }
        for (int j = 0; j < buttonCount; ++j) {
            buttons[j] = SDL_JoystickGetButton(sdl_joystick, j);
        }
    }
}


/*
    accessors
*/

// get axis value
float Joystick::axesState(int axis) {
    if (present) {
        return axes[axis];
    }

    return -1;
}

// get button state
unsigned char Joystick::buttonState(int button) {
    if (present) {
        return buttons[button];
    }
    return SDL_RELEASED;
}

// get number of axes
int Joystick::getAxesCount() {
    return axesCount;
}

// get number of buttons
int Joystick::getButtonCount() {
    return buttonCount;
}

// return if joystick present
bool Joystick::isPresent() {
    return present;
}

// get name of joystick
const char* Joystick::getName() {
    return name;
}
