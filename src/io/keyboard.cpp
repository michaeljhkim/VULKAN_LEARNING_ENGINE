#include "keyboard.hpp"

// std
#include <limits>

/*
	Static variables
*/

// Global variables to store keyboard states
constexpr int KEY_COUNT = 512;			// Get the number of keys - Maximum number of key states
bool Keyboard::keys[KEY_COUNT] = { 0 };
bool Keyboard::keysChanged[KEY_COUNT] = { 0 };

/*
	Static callback
*/
void Keyboard::keyCallback(const SDL_Event& event) noexcept {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Scancode keyPressed = event.key.keysym.scancode;
        bool isDown = (event.type == SDL_KEYDOWN);

        // Update key state and change status
        if (!event.key.repeat) {
            bool& currentState = keys[keyPressed];
            keysChanged[keyPressed] = (currentState != isDown);
            currentState = isDown;
        }
    }
}


/*
	Static accessors
*/
bool Keyboard::key(SDL_Scancode input_key) { return keys[static_cast<int>(input_key)]; }

bool Keyboard::keyChanged(SDL_Scancode input_key) noexcept {
    bool& changed = keysChanged[static_cast<int>(input_key)];
    bool wasChanged = changed;
    changed = false; // Reset change status
    return wasChanged;
}

bool Keyboard::keyWentDown(SDL_Scancode input_key) { return key(input_key) && keyChanged(input_key); }
bool Keyboard::keyWentUp(SDL_Scancode input_key) { return !key(input_key) && keyChanged(input_key); }