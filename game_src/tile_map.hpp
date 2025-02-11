#ifndef _TILEMAP_HPP
#define _TILEMAP_HPP

#define CELLS_PER_CHUNK 16
#define CELL_WIDTH 20.0f

#include "../include/raylib.h"

enum CellFlags 
{
    CF_InUse = 0x01
};

struct gridCell 
{
    int directionIndex;
    unsigned int cellFlags;
};

struct gridChunk
{
    struct  { int x; int y;} chunkCoords; // lower left location of chunks    
    gridCell cells[CELLS_PER_CHUNK][CELLS_PER_CHUNK];
    
    unsigned int chunkFlags;
    gridChunk *nextInHash;
};

#define CHUNK_ACTIVE_FLAG 0x00000001

#define CHUNK_ACTIVE(chunk) ((chunk->chunkFlags) & CHUNK_ACTIVE_FLAG) > 0
#define SET_CHUNK_ACTIVE(chunk) ((chunk->chunkFlags) |= CHUNK_ACTIVE_FLAG)
#define SET_CHUNK_INACTIVE(chunk) ((chunk->chunkFlags) &= ~CHUNK_ACTIVE_FLAG)

#define CHUNK_LIST_LENGTH(chunk) (((chunk->chunkFlags) >> 8) & 0x000000ff)
typedef void (*CellCallback) (gridCell & cell);
typedef void (*ChunkCallback) (gridChunk & chunk);
struct chunkCoordinate
{
    int x;
    int y;
};

struct tile_map 
{ 
    chunkCoordinate activeChunks[64];
    unsigned int numActiveChunks;
    gridChunk chunks[64];
};


int getChunkIndex(int x, int y) ;
gridChunk *newChunk(int x, int y) ;
gridChunk *initializeChunk(int x, int y, tile_map & map) ;
bool mapContainsChunk(int x, int y, tile_map & map) ;
gridChunk * getChunk(int x, int y, tile_map & map) ;
gridChunk *getChunk(Vector3 worldPosition, tile_map & map);


#define IS_CELL_FREE(cell) ((((cell).cellFlags) & (CellFlags::CF_InUse)) == 0)

void foreach_active_chunk(ChunkCallback callback, tile_map & map);
void chunk_init(gridChunk * chunk);
Vector3 cellPositionInChunk(int i, int j, gridChunk & chunk); 
Vector3 chunkCenter(gridChunk & chunk) ;
Vector3 chunkDimensions(gridChunk & chunk) ;
void foreach_cell(CellCallback callback, gridChunk & chunk);
void init_tilemap(tile_map & map);

#endif