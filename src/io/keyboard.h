#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <cstring>
#include <iostream>
#include <string>

/*
    keyboard class to handle keyboard callbacks
*/

class Keyboard {
public:
    /*
        static callback
    */
    //Keyboard() {}

    // key state changed
    static void keyCallback(SDL_Event& keyboard_event);

    /*
        static accessors
    */

    // get key state
    static bool key(int key);

    // get if key recently changed
    static bool keyChanged(int key);

    // get if key recently changed and is up
    static bool keyWentUp(int key);

    // get if key recently changed and is down
    static bool keyWentDown(int key);

private:
    /*
        static keyboard values
    */

    // key state array (true for down, false for up)
    static bool keys[];

    // key changed array (true if changed)
    static bool keysChanged[];
};

#endif