#ifndef _PATHFINDING_HPP
#define _PATHFINDING_HPP
#include "tile_map.hpp"

#define INDEX_IN_BOUNDS(p) p.x >= 0 && p.x < CELLS_PER_CHUNK && p.y >= 0 && p.y < CELLS_PER_CHUNK

struct vec2int 
{ 
    int x; 
    int y;
};

struct vec2uint 
{ 
    unsigned int i;
    unsigned int j; 
};

struct pathfinding_data
{
    int distanceToTarget;
    bool processed;
    bool waitingForProcessing;
    vec2int moveDir;
};


void process_cell(unsigned int i, unsigned int j, gridChunk * chunk);
void run_pathfinder(unsigned int targetX, unsigned int targetY, gridChunk * chunk);
pathfinding_data *getPathfindingData(unsigned int x, unsigned int y) ;
pathfinding_data *getPathfindingData(Vector3 position);


#endif