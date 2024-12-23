# YURRGOHT_ENGINE

(I have decided to start over using another project as a base, this is now purely for archiving, also changed repo name at the top for less confusion, but this was the original YURRGOHT ENGINE)

This is my (second) attempt at creating a C++ game engine. My old goal was to continue with openGL, I am currently switching to vulkan. I am pretty sure the deprecation of the whole API was not going to be worth it. This is primarily designed for linux systems. 
This second attempt is based off of the tutorial of Michael Grieco :
- Tutorial Playlist: https://www.youtube.com/playlist?list=PLysLvOneEETPlOI_PI4mJnocqIpr2cSHS 
- github repo: https://github.com/michaelg29/yt-tutorials.

I noticed that it had most of the starting features I wanted to implement, it had better structure, and was just easier to port that code than to create from the ground up. Most likely, the code will become unrecognizeable as I keep adding and changing to suite my goals. 
It had shaders, models, simple lighting sytem, user input, physics, collisions, textures, skybox, and fonts.   

##

The goal of this engine is NOT to create the most advanced, best looking engine out there. Instead, here are the current goals:

- Highly optimized (not good at that part yet)
- Modular: easy to take apart and put back together aspects (also maybe make it accept libraries that might actually make it more advanced like unreal something)
- Mod-able: hopefully people can more easily build on top of the games being made here
- Retro-style: make it look and feel like late 90's, early 2000's 3D games
- Optional higher fidelity options: nothing too crazy, but it will be figured out as I go
- Randomly Generated world system that is not repetitive feeling
- Decent enemy AI
- Accomodate 2D sprite based games (think Doom or Duke nukem 3D)
- templates that don't suck, and people can actually work with! (like how in GZDoom the default Doom code is an excellent starting point)

- Learning how to make a decent game engine for personal useage

##

On the note of moddablity, the hope is creating a scripting language way down the line that also has client-side multiplayer capbalities.
Like GZDoom, but the issue there is getting custom functions to work. This is still way down the line.

##

IMMEDIATE TODO LIST (might as well just put it here):
- convert from opengl to vulkan
- make the engine run primarily with async, so input will always feels smooth regardless of framerates
- possibly figure out better name for this game engine

##

DONE LIST : 
- ported tutorial code to linux
- switched from GLFW to SDL2
- switched from Visual Studio SLN to CMAKE
- converted shader code to openGL 4.6 (only had to change a couple of lines)
- figure out how the light system works
- figure out how the physics works
- figure out how the collisions works
- figure out how the skybox works
- (maybe) figure out dynamic shaders
 

