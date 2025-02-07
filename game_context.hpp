#ifndef _GAME_CONTEXT_HPP
#define _GAME_CONTEXT_HPP

#include "include/raylib.h"
#include "include/raymath.h"


struct GameContext 
{ 
    int screenWidth;
    int screenHeight;
    int renderWidth;
    int renderHeight;

    Vector2 MousePosition;
    Vector2 MouseDelta;
    Vector2 MouseScroll;
    Camera camera;
    float applicationTime;
    float deltaTime;
    bool running;
};


#endif