#pragma once

#include <SDL.h>

//namespace lve {
class Keyboard {
 public:
	struct KeyMappings {
		SDL_Scancode moveForward = SDL_SCANCODE_W;
		SDL_Scancode moveBackward = SDL_SCANCODE_S;
		SDL_Scancode moveLeft = SDL_SCANCODE_A;
		SDL_Scancode moveRight = SDL_SCANCODE_D;
		
		SDL_Scancode moveUp = SDL_SCANCODE_SPACE;
		SDL_Scancode moveDown = SDL_SCANCODE_LCTRL;

		//SDL_Scancode lookLeft = GLFW_KEY_LEFT;
		//SDL_Scancode lookRight = GLFW_KEY_RIGHT;
		//SDL_Scancode lookUp = GLFW_KEY_UP;
		//SDL_Scancode lookDown = GLFW_KEY_DOWN;
	};
	/*
			static callback
	*/

	// key state changed
	static void keyCallback(const SDL_Event& event);

	/*
			static accessors
	*/

	// get key state 
	static bool key(SDL_Scancode key);

	//get key changes
	static bool keyChanged(SDL_Scancode key);
	static bool keyWentUp(SDL_Scancode key);
	static bool keyWentDown(SDL_Scancode key);

	static const KeyMappings key_mappings;

private:
	// key state array (true for down, false for up)
	static bool keys[];

	// key changed array (true if changed)
	static bool keysChanged[];
};

//}	// namespace lve