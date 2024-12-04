#pragma once

#include <SDL2/SDL.h>
#include "../graphics/game_object.hpp"
#include "../graphics/vulkan_window.hpp"

//namespace lve {
class KeyboardMovementController {
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
	//Keyboard() {}

	// key state changed
	static void keyCallback(SDL_Event keyboard_event);

	/*
			static accessors
	*/

	// get key state
	static bool key(SDL_Scancode key);

	// get if key recently changed
	static bool keyChanged(SDL_Scancode key);

	// get if key recently changed and is up
	static bool keyWentUp(SDL_Scancode key);

	// get if key recently changed and is down
	static bool keyWentDown(SDL_Scancode key);


	void moveInPlaneXZ(float dt, GameObject& gameObject);

	KeyMappings key_mappings{};
	float moveSpeed{3.f};
	float lookSpeed{1.5f};

private:
	/*
			static keyboard values
	*/

	// key state array (true for down, false for up)
	static bool keys[];

	// key changed array (true if changed)
	static bool keysChanged[];
};

//}	// namespace lve