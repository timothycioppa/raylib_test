#include "game_state.hpp"
#include <stdio.h>

#define _ASSERT(e, m) if (!(e)) { printf("%s\n", m); fflush(stdout); *((int*)0) = 0;}
GameState *_states[E_GameState::GS_COUNT];
E_GameState gCurrentState = E_GameState::GS_COUNT;


void request_state_change(E_GameState newState) 
{    
    _ASSERT(newState != E_GameState::GS_COUNT, "request_state_change: cannot request state change to no state");    
    GameState *currentState = _states[int(gCurrentState)];
    _ASSERT(currentState != nullptr, "request_state_change: cannot request state change when current state is null");
    currentState->NextState = newState;  
}


void register_state(E_GameState name, GameState *state)
{
    _ASSERT(name != E_GameState::GS_COUNT, "register_state: cannot register state with GS_COUNT");
    _ASSERT(state != nullptr, "register_state: cannot register null state");
    _states[int(name)] = state;
}


GameState *get_current_state() 
{
    _ASSERT(gCurrentState != E_GameState::GS_COUNT, "get_current_state: gCurrentState cannot be GS_COUNT");  
    GameState *result = _states[int(gCurrentState)];
    _ASSERT(result != nullptr, "get_current_state: current state cannot be null");   
    return result;
}


void set_initial_game_state(E_GameState state, GameContext & context)
{ 
    _ASSERT(state != E_GameState::GS_COUNT, "set_initial_game_state: state can't be GS_COUNT");
    gCurrentState = state;
    GameState* startState = _states[int(gCurrentState)];
    _ASSERT(startState != nullptr, "set_initial_game_state: initial state cannot be null");
    startState->OnStateEntry(context);
}

void check_for_state_change(GameContext & context) 
{
    GameState *current = get_current_state();

    if ((current->NextState != E_GameState::GS_COUNT) && (current->NextState != gCurrentState))
    {
        current->OnStateExit(context);
        gCurrentState = current->NextState;
        GameState *newState = _states[int(gCurrentState)];        
        current->NextState = E_GameState::GS_COUNT;        
        newState->OnStateEntry(context);
    }
}

#undef _ASSERT