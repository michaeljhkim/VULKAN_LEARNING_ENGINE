#include "mouse.hpp"
#include "camera.hpp"

//NOTE: SDL2 provides their own mouse deltas, so no need to store total x and y mouse positions

/*
    define initial static values
*/
double Mouse::dx = 0;
double Mouse::dy = 0;
double Mouse::scrollDx = 0;
double Mouse::scrollDy = 0;
bool Mouse::firstMouse = true;

//SDL_BUTTON_X2 is last mouse button
//SDL_BUTTON_X2 + 1 because the 0th element will always be empty
bool Mouse::buttons[SDL_BUTTON_X2 + 1] = { 0 };
bool Mouse::buttonsChanged[SDL_BUTTON_X2 + 1] = { 0 };



/*
    callbacks
*/

// cursor position changed - Accumulate movement deltas for smooth movement
void Mouse::cursorPosCallback(const SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION) {
        dx += event.motion.xrel;  // Update relative X movement
        dy -= event.motion.yrel;  // Update relative Y movement (negated for typical camera usage)
    }
}

// Scroll wheel moved - Accumulate scroll deltas for smooth scrolling
void Mouse::mouseWheelCallback(const SDL_Event& event) {
    if (event.type == SDL_MOUSEWHEEL) {
        scrollDx += event.wheel.x; 
        scrollDy += event.wheel.y;
    }
}

// Mouse button state changed
void Mouse::mouseButtonCallback(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        int buttonPressed = event.button.button;
        bool isPressed = (event.type == SDL_MOUSEBUTTONDOWN);                   // Determine if the button state changed
        buttonsChanged[buttonPressed] = (buttons[buttonPressed] != isPressed);  // Record state change
        buttons[buttonPressed] = isPressed;                                     // Update current state
    }
}


// Mouse Button is being repeated (held down)
//Since SDL2 does not provide a way to check if a button is repeating, we do this check
void Mouse::mouseButtonRepeat() {
    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);    //Update button states for common mouse buttons
    for (int button : {SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT}) {
        buttons[button] = (mouseState & SDL_BUTTON(button)); }
}


/*
    accessors
*/

// Return if a button's state has changed, then reset the changed state
bool Mouse::buttonChanged(Uint8 button) {
    bool ret = buttonsChanged[button];
    buttonsChanged[button] = false; // Reset after reading
    return ret;
}

// Return if a button's state has changed and is up or down
bool Mouse::buttonWentUp(Uint8 button) { return !buttons[button] && buttonChanged(button); }
bool Mouse::buttonWentDown(Uint8 button) { return buttons[button] && buttonChanged(button); }