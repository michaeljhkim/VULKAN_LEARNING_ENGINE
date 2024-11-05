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

//const Uint8* keys_state = SDL_GetKeyboardState(NULL);

/*
//The only thing this constructor needs to do is to just 
Keyboard::Keyboard() {
    //memset(previous_keys_state, 0, keys_size);
}
*/

/*
    static callback
*/

// key state changed
// We check if every event is account for
void Keyboard::keyCallback(SDL_Event& event) {
    while( SDL_PollEvent(&event) != 0 ) {
        SDL_Keycode keyPressed = event.key.keysym.sym;
        if (event.type != SDL_KEYUP) {
            if (!keys[keyPressed]) {
                keys[keyPressed] = true;
            }
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
bool Keyboard::key(int key) {
    return keys[key];
}

// get if key recently changed
bool Keyboard::keyChanged(int key) {
    bool ret = keysChanged[key];
    // set to false because change no longer new
    keysChanged[key] = false;
    return ret;
}

// get if key recently changed and is up
bool Keyboard::keyWentDown(int key) {
    return keys[key] && keyChanged(key);
}

// get if key recently changed and is down
bool Keyboard::keyWentUp(int key) {
    return !keys[key] && keyChanged(key);
}