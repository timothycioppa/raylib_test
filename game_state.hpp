#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_

#include "game_context.hpp"

enum E_GameState 
{
    GS_MAIN = 0,
    GS_COUNT = 1
};

typedef void (*GameStateProc)(GameContext & context);

struct GameState 
{
    GameStateProc OnStateEntry;
    GameStateProc OnStateExit;
    GameStateProc OnUpdate;
    GameStateProc OnRender;
    E_GameState NextState;
};

#define STATE_ENTER(name)   void name##_Start (GameContext & context) 
#define STATE_EXIT(name)     void name##_Stop (GameContext & context) 
#define STATE_UPDATE(name)  void name##_Update (GameContext & context) 
#define STATE_RENDER(name)  void name##_Render (GameContext & context) 

#define EXPORT_STATE(name) GameState name##_State { \
      name##_Start, \
      name##_Stop, \
      name##_Update, \
      name##_Render, \
      E_GameState::GS_COUNT \
}\

#define GET_STATE(name) name##_State
#define DECLARE_STATE(name) extern GameState name##_State

void request_state_change(E_GameState newState) ;
void register_state(E_GameState name, GameState *state);
GameState *get_current_state() ;
void set_initial_game_state(E_GameState state, GameContext & context);
void check_for_state_change(GameContext & context) ;

#endif