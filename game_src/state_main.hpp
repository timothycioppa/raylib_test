#ifndef _STATE_MAIN_HPP_
#define _STATE_MAIN_HPP_

#include "../game_state.hpp"
#include "../game_context.hpp"

void StartGame(GameContext & gameContext);
void EndGame(GameContext & gameContext);
void UpdateGame(GameContext & gameContext);
void RenderGame(GameContext & gameContext);

DECLARE_STATE(MAIN);

#endif