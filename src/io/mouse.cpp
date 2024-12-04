#include "mouse.hpp"
#include "camera.h"

/*
    define initial static values
*/
// change in x position (delta x)
double Mouse::dx = 0;
// change in y position (delta y)
double Mouse::dy = 0;

// change in scroll x
double Mouse::scrollDx = 0;
// change in scroll y
double Mouse::scrollDy = 0;

// if this is the first change in the mouse position
bool Mouse::firstMouse = true;

//SDL_BUTTON_X2 is last mouse button
//SDL_BUTTON_X2 + 1 because the 0th element will always be empty

// button state array (true for down, false for up)
bool Mouse::buttons[SDL_BUTTON_X2 + 1] = { 0 };
// button changed array (true if changed)
bool Mouse::buttonsChanged[SDL_BUTTON_X2 + 1] = { 0 };

//Since SDL2 does not provide a way to check if a button is repeating, we store the old inputs
bool buttonsPrevious[SDL_BUTTON_X2 + 1] = { 0 };

/*
    callbacks
*/

// cursor position changed
void Mouse::cursorPosCallback(SDL_Event event) {
    //if the mouse is moved at all, RELATIVE x and y values are passed into the camera MouseLook function
    if(event.type == SDL_MOUSEMOTION) {
        dx += event.motion.xrel;
        dy -= event.motion.yrel;
    }
}


// mouse button state changed
void Mouse::mouseButtonCallback(SDL_Event event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        int buttonPressed = event.button.button;

        // Update button state based on the event type
        bool isPressed = (event.type == SDL_MOUSEBUTTONDOWN);
        buttonsChanged[buttonPressed] = (buttons[buttonPressed] != isPressed); // Check for state change

        buttons[buttonPressed] = isPressed;
        buttonsPrevious[buttonPressed] = isPressed; // Update previous state
    }
}


// Mouse Button is being repeated (held down)
// I will figure out where I can use this later though
void Mouse::mouseButtonRepeat() {
    // Poll mouse button state
    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);

    // Array of mouse buttons to check
    const int mouseButtons[] = {SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT};

    for (int button : mouseButtons) {
        buttons[button] = (mouseState & SDL_BUTTON(button));
    }
}

// scroll wheel moved
void Mouse::mouseWheelCallback(SDL_Event event) {
    if (event.type == SDL_MOUSEWHEEL) {
        scrollDx = event.wheel.x;
        scrollDy = event.wheel.y;
    }
}

/*
    accessors
*/

// Helper function to get and reset a value
template <typename T>
T Mouse::getAndReset(T& value) {
    T temp = value;
    value = 0;
    return temp;
}

// get mouse change in x
double Mouse::getDX() {
    return getAndReset(dx);
}

// get mouse change in y
double Mouse::getDY() {
    return getAndReset(dy);
}

// get scroll value in x
double Mouse::getScrollDX() {
    return getAndReset(scrollDx);
}

// get scroll value in y
double Mouse::getScrollDY() {
    return getAndReset(scrollDy);
}

// Return the state of a mouse button
bool Mouse::button_state(Uint8 button) {
    return buttons[button];
}

// Return if a button's state has changed, then reset the changed state
bool Mouse::buttonChanged(Uint8 button) {
    bool ret = buttonsChanged[button];
    buttonsChanged[button] = false; // Reset after reading
    return ret;
}

// Return if a button's state has changed and is now up
bool Mouse::buttonWentUp(Uint8 button) {
    return !button_state(button) && buttonChanged(button);
}

// Return if a button's state has changed and is now down
bool Mouse::buttonWentDown(Uint8 button) {
    return button_state(button) && buttonChanged(button);
}