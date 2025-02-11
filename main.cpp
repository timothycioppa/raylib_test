#include "g_main.hpp"
#include "scratch.hpp"
const int screenWidth = 1600;
const int screenHeight = 900;
#define SCRATCH_SIZE 4 * 1024 * 1024
unsigned char scratchArena[SCRATCH_SIZE];
unsigned char* scratchArenaHead;
unsigned int used = 0;



int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);    // Window configuration flags    
    InitWindow(screenWidth, screenHeight, "Josel Cioppa Game");   
    SetExitKey(KEY_NULL);   
    SetTargetFPS(120);                  
    scratch_reset();
 
    GameContext mainContext;
    InitializeGame(mainContext);

    while (mainContext.running)    
    {
        scratch_reset();
        
        ProcessFrame(mainContext);

        if (WindowShouldClose()) 
        {
            mainContext.running = false;
        }
    }

    DestroyGame(mainContext);
    CloseWindow();                 
  
    return 0;
}


void* scratch_alloc(unsigned int size) 
{
    // cannot allocate enough
    if (used + size >= SCRATCH_SIZE) 
    {
        return 0;
    }

    unsigned char* result = scratchArenaHead;
    scratchArenaHead += size;
    used += size;
    return (void*) (result);
}


void scratch_reset(void) 
{
    used = 0;
    scratchArenaHead = scratchArena;
}