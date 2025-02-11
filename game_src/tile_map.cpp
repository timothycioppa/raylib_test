#include "tile_map.hpp"
#include <math.h>
#include <string.h>

Vector3 chunkCenter(gridChunk & chunk) 
{
    float cellSize = CELL_WIDTH;
    float chunkSize = float(CELLS_PER_CHUNK) * cellSize;
    float halfChunkSize = 0.5f * chunkSize;
    float startX = chunk.chunkCoords.x * chunkSize;
    float startY = chunk.chunkCoords.y * chunkSize;    

    return {
        startX + halfChunkSize,
        0.0f, 
        startY + halfChunkSize
    };
}

Vector3 chunkDimensions(gridChunk & chunk) 
{
    float cellSize = CELL_WIDTH;
    float chunkSize = float(CELLS_PER_CHUNK) * cellSize;
    return {chunkSize, chunkSize, chunkSize};
}

Vector3 cellPositionInChunk(int i, int j, gridChunk & chunk) 
{    
    float cellSize = CELL_WIDTH;
    float halfCellSize = cellSize * 0.5f;
    float chunkSize = float(CELLS_PER_CHUNK) * cellSize;
  
    float startX = chunk.chunkCoords.x * chunkSize;
    float startY = chunk.chunkCoords.y * chunkSize;
   
    return 
    {
        startX + float(i) * cellSize + halfCellSize,
        5.0f, 
        startY + float(j) * cellSize + halfCellSize 
    };
}

void chunk_init(gridChunk * chunk) {

    for (int i = 0; i < CELLS_PER_CHUNK; i++) 
    {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) 
        {
            gridCell & cell = chunk->cells[i][j];
            cell.cellFlags = 0; 
        }
    }
}

void foreach_cell(CellCallback callback, gridChunk & chunk)
{
    for (int i = 0; i < CELLS_PER_CHUNK; i++) 
    {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) 
        {
            gridCell & cell = chunk.cells[i][j];
            callback(cell);
        }
    }
}

gridChunk chunkArena[64];
int chunkArenaHead = 0;

int getChunkIndex(int x, int y) 
{
    int hashValue = 3 * x + 11 * y;
    int hashBucket = hashValue & 63;
    return hashBucket; 
}

gridChunk *newChunk(int x, int y) 
{
    gridChunk *newChunk = chunkArena + chunkArenaHead;
    chunkArenaHead++;
    newChunk->chunkCoords = {x, y};
    newChunk->nextInHash = 0;
    return newChunk;
}

gridChunk *initializeChunk(int x, int y, tile_map & map) 
{
    int chunkIndex = getChunkIndex(x,y);
    gridChunk *currentChunk = map.chunks + chunkIndex;
    gridChunk *prev = 0;

    // base chunk hasn't been initialized
    if (!CHUNK_ACTIVE(currentChunk)) 
    {
        SET_CHUNK_ACTIVE(currentChunk);
        memset(currentChunk->cells, 0, sizeof(currentChunk->cells));
   
        currentChunk->chunkCoords = { x, y};
        currentChunk->nextInHash = 0;
   
        map.activeChunks[map.numActiveChunks] = {x, y};
        map.numActiveChunks++;

        return currentChunk;
    } 

    // already initialized
    while (currentChunk) 
    {   
        if (currentChunk->chunkCoords.x == x && currentChunk->chunkCoords.y == y) 
        {
            return currentChunk;
        }

        prev = currentChunk;
        currentChunk = currentChunk->nextInHash;
    }

    if (!currentChunk) 
    {        
        currentChunk = newChunk(x, y);
        memset(currentChunk->cells, 0, sizeof(currentChunk->cells));
        prev->nextInHash = currentChunk;
        map.activeChunks[map.numActiveChunks] = {x, y};
        map.numActiveChunks++;
        return currentChunk;
    }

    return currentChunk;
}

void foreach_active_chunk(ChunkCallback callback, tile_map & map) 
{
    for (int i = 0; i < map.numActiveChunks; i++) 
    {
        chunkCoordinate coords = map.activeChunks[i];
        gridChunk *pchunk = getChunk(coords.x, coords.y, map);        
        if (pchunk) 
        {
            callback(*pchunk);
        }
    }
}

bool mapContainsChunk(int x, int y, tile_map & map) 
{
    int chunkIndex = getChunkIndex(x,y);
    gridChunk *chunk = map.chunks + chunkIndex;
    
    if (!CHUNK_ACTIVE(chunk)) 
    {
        return false;
    }

    while (chunk) 
    {
        // exact match
        if(chunk->chunkCoords.x == x && chunk->chunkCoords.y == y) 
        {
            return true;
        }

        chunk = chunk->nextInHash;
    }

    return false;
}

gridChunk * getChunk(int x, int y, tile_map & map) 
{
    int chunkIndex = getChunkIndex(x,y);
    gridChunk *chunk = map.chunks + chunkIndex;
    
    if (!CHUNK_ACTIVE(chunk)) 
    {
        return 0;
    }

    while (chunk) 
    {
        // exact match
        if(chunk->chunkCoords.x == x && chunk->chunkCoords.y == y) 
        {
            return chunk;
        }
        
        chunk = chunk->nextInHash;
    }

    return 0;
}


gridChunk *getChunk(Vector3 worldPosition, tile_map & map) 
{
    float cellSize = CELL_WIDTH;
    float chunkSize = float(CELLS_PER_CHUNK) * cellSize;

    int x = (int) floorf(worldPosition.x / chunkSize);
    int y = (int) floorf(worldPosition.x / chunkSize);

    return getChunk(x, y, map);

}

#include <string.h>
#include <stdio.h>

void init_tilemap(tile_map & map) 
{
    chunkArenaHead = 0;
    memset(chunkArena, 0, sizeof(chunkArena));
    map.numActiveChunks = 0;
    memset(map.activeChunks, 0, sizeof(map.activeChunks));
    memset(map.chunks, 0, sizeof(map.chunks));
}