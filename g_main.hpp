#ifndef _G_MAIN_HPP
#define _G_MAIN_HPP

#include "game_context.hpp"

void InitializeGame(GameContext & gameState) ;
void ProcessFrame(GameContext & gameContext) ;
void ProcessInput(GameContext & gameContext) ;
void DestroyGame(GameContext & context) ;
void OnWindowResize(GameContext & context) ;

#endif