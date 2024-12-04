#include "keyboard.h"
#include <stdio.h>
#include <iostream>
#include <string>

/*
    define initial static values
*/

// key state array (true for down, false for up)
//bool Keyboard::keys[GLFW_KEY_LAST] = { 0 };
// Get the number of keys (usually at least 512)
const int KEY_COUNT = 512; // Maximum number of key states

// Global variables to store keyboard states
bool Keyboard::keys[KEY_COUNT] = { 0 };

// key changed array (true if changed)
bool Keyboard::keysChanged[KEY_COUNT] = { 0 };


/*
    static callback
*/

// key state changed
// We check if every event is account for
void Keyboard::keyCallback(SDL_Event event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        SDL_Scancode keyPressed = event.key.keysym.scancode;
        if (event.type == SDL_KEYDOWN) {
            if (!keys[keyPressed])
                keys[keyPressed] = true;
        }
        else {
            keys[keyPressed] = false;
        }
        keysChanged[keyPressed] = (!event.key.repeat);
    }
}

/*
    static accessors
*/

// get key state
bool Keyboard::key(SDL_Scancode key) {
    int keyint = static_cast<int>(key);
    return keys[keyint];
}

// get if key recently changed
bool Keyboard::keyChanged(SDL_Scancode key) {
    //key is of type SDL_Keycode which is an enum. Must cast to an int
    int keyint = static_cast<int>(key);
    bool ret = keysChanged[keyint];

    // set to false because change no longer new
    keysChanged[keyint] = false;
    return ret;
}

// get if key recently changed and is up
bool Keyboard::keyWentDown(SDL_Scancode key) {
    int keyint = static_cast<int>(key);
    return keys[key] && keyChanged(key);
}

// get if key recently changed and is down
bool Keyboard::keyWentUp(SDL_Scancode key) {
    int keyint = static_cast<int>(key);
    return !keys[keyint] && keyChanged(key);
}