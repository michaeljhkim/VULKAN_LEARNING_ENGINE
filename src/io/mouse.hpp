#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <memory>
#include <utility>
#include "../graphics/game_object.hpp"

/*
    mouse class to handle mouse callbacks
*/

class Mouse {
public:
    /*
        callbacks
    */

    // Change in cursor position, mouse buttons, and mouse wheel
    static void cursorPosCallback(const SDL_Event& event);
    static void mouseButtonCallback(const SDL_Event& event);
    static void mouseWheelCallback(const SDL_Event& event);

    // any mouse button is being held down
    static void mouseButtonRepeat();

    /*
        accessors
    */
    // Get member variables 
    static double getDX() noexcept { return std::exchange(dx, 0.0); }
    static double getDY() noexcept { return std::exchange(dy, 0.0); }
    static double getScrollDX() noexcept { return std::exchange(scrollDx, 0.0); }
    static double getScrollDY() noexcept { return std::exchange(scrollDy, 0.0); }

    // return if button changed then reset it in the changed array
    static bool buttonChanged(Uint8 button);
    static bool buttonWentUp(Uint8 button);
    static bool buttonWentDown(Uint8 button);

private:
    /*
        static mouse values
    */

    // x and y positon
    //static double x;
    //static double y;

    // previous x and y position
    static double lastX;
    static double lastY;

    // change in x position from lastX and y position from lastY
    static double dx;
    static double dy;

    // change in scroll x and y
    static double scrollDx;
    static double scrollDy;

    // if this is the first change in the mouse position
    static bool firstMouse;

    // button state array (true for down, false for up)
    static bool buttons[];
    // button changed array (true if changed)
    static bool buttonsChanged[];
};
