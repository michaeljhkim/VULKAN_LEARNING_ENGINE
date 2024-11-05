#include "mouse.h"
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

// button state array (true for down, false for up)
bool Mouse::buttons[GLFW_MOUSE_BUTTON_LAST] = { 0 };
// button changed array (true if changed)
bool Mouse::buttonsChanged[GLFW_MOUSE_BUTTON_LAST] = { 0 };

//Since SDL2 does not provide a way to check if a button is repeating, we store the old inputs
bool buttonsPrevious[GLFW_MOUSE_BUTTON_LAST] = { 0 };

/*
    callbacks
*/

// cursor position changed
void Mouse::cursorPosCallback(SDL_Event& event) {
    while( SDL_PollEvent(&event) != 0 ) {
        //if the mouse is moved at all, RELATIVE x and y values are passed into the camera MouseLook function
        if(event.type == SDL_MOUSEMOTION) {
            dx += event.motion.xrel;
            dy += event.motion.yrel;
        }
    }
}

// mouse button state changed
void Mouse::mouseButtonCallback(SDL_Event& event) {
    while( SDL_PollEvent(&event) != 0 ) {
        SDL_Keycode buttonPressed = event.button.button;
        if (event.type != SDL_MOUSEBUTTONUP) {
            if (!buttons[buttonPressed]) {
                buttons[buttonPressed] = true;
            }
        }
        else {
            buttons[buttonPressed] = false;
        }
        //check if the current button was pressed previously as well, then story in the buttonsChanged (delta) 
        buttonsChanged[buttonPressed] = (buttons[buttonPressed] != buttonsPrevious[buttonPressed]);
        buttonsPrevious[buttonPressed] = buttons[buttonPressed];
    }

}

// scroll wheel moved
void Mouse::mouseWheelCallback(SDL_Event& event) {
    scrollDx = event.wheel.x;
    scrollDy = event.wheel.y;
}

/*
    accessors
*/

// get mouse change in x
double Mouse::getDX() {
    double _dx = dx;
    // set to 0 because change no longer new
    dx = 0;
    return _dx;
}

// get mouse change in y
double Mouse::getDY() {
    double _dy = dy;
    // set to 0 because change no longer new
    dy = 0;
    return _dy;
}

// get scroll value in x
double Mouse::getScrollDX() {
    double _scrollDx = scrollDx;
    // set to 0 because change no longer new
    scrollDx = 0;
    return _scrollDx;
}

// get scroll value in y
double Mouse::getScrollDY() {
    double _scrollDy = scrollDy;
    // set to 0 because change no longer new
    scrollDy = 0;
    return _scrollDy;
}

// get button state
bool Mouse::button(int button) {
    return buttons[button];
}

// return if button changed then reset it in the changed array
bool Mouse::buttonChanged(int button) {
    bool ret = buttonsChanged[button];
    // set to false because change no longer new
    buttonsChanged[button] = false;
    return ret;
}

// return if button changed and is now up
bool Mouse::buttonWentUp(int button) {
    return !buttons[button] && buttonChanged(button);
}

// return if button changed and is now down
bool Mouse::buttonWentDown(int button) {
    return buttons[button] && buttonChanged(button);
}