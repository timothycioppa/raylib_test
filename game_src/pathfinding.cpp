#include "pathfinding.hpp"

#define ASSRT(e) if (!(e)) { *((int*)0) = 0;}
vec2uint indexQueue[CELLS_PER_CHUNK * CELLS_PER_CHUNK];
unsigned int index_head = 0;
unsigned int index_tail = 0;
int amountToProcess = 0;
pathfinding_data pf_data[CELLS_PER_CHUNK][CELLS_PER_CHUNK];

pathfinding_data *getPathfindingData(unsigned int x, unsigned int y) 
{
    if (x >= 0 && x < CELLS_PER_CHUNK && y >= 0 && y < CELLS_PER_CHUNK) 
    {
        return &pf_data[x][y];
    }
    
    return 0;
}

pathfinding_data *getPathfindingData(Vector3 position) 
{  
    float cellSize = CELL_WIDTH;
    float chunkSize = cellSize * float(CELLS_PER_CHUNK);     
    pathfinding_data *result = 0;

    if (position.x >= 0 && position.x <= chunkSize && position.z >= 0 && position.z < chunkSize)
    {
        unsigned int playerI = int(position.x) / int(CELL_WIDTH);
        unsigned int playerJ = int(position.z) / int(CELL_WIDTH);
        result =  getPathfindingData(playerI, playerJ);
    }

    return result; 
}

void process_cell(unsigned int i, unsigned int j, gridChunk * chunk) 
{
    ASSRT(IS_CELL_FREE(chunk->cells[i][j]))
  
    int currentDistance = (pf_data[i][j]).distanceToTarget;
    pf_data[i][j].processed = true;
    pf_data[i][j].waitingForProcessing = false;
    amountToProcess--;
    
    #define PROCESS_CELL(I,J) if (IS_CELL_FREE(chunk->cells[I][J]))  { \
        pathfinding_data & pfd = pf_data[I][J]; \
        if (!pfd.processed && !pfd.waitingForProcessing)  { \
            pfd.waitingForProcessing = true;\
            amountToProcess++; \
            pfd.distanceToTarget = currentDistance + 1; \
            indexQueue[index_head] = {I, J}; \
            index_head++; }} \
            

    if (i > 0)  { PROCESS_CELL(i-1, j) }
    if (i < (CELLS_PER_CHUNK - 1)) { PROCESS_CELL(i+1, j) }
    if (j > 0)  { PROCESS_CELL(i, j-1) }
    if (j < (CELLS_PER_CHUNK - 1)) { PROCESS_CELL(i, j + 1) }

    #undef PROCESS_CELL

}

vec2int offsets[8] = 
{
    {1,   0},
    {1,   1},
    {0,   1},
    {-1,  1},
    {-1,  0},
    {-1, -1},
    {0,  -1},
    {1,  -1}
};

void run_pathfinder(unsigned int targetX, unsigned int targetY, gridChunk * chunk) 
{
    amountToProcess = 0;
    index_head = 0;
    index_tail = 0;
    
    // initialize pathfinding data, nothing is processed and no distance is valid
    for (int i = 0; i < CELLS_PER_CHUNK; i++) 
    {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) 
        {
            pf_data[i][j].distanceToTarget = -1;
            pf_data[i][j].processed = false;
            pf_data[i][j].waitingForProcessing = false;
            pf_data[i][j].moveDir = {0,0};
        }
    }

    if (!IS_CELL_FREE(chunk->cells[targetX][targetY])) 
    {
        return;
    }

    (pf_data[targetX][targetY]).distanceToTarget = 0;

    process_cell(targetX, targetY, chunk); 

    int numCells = CELLS_PER_CHUNK * CELLS_PER_CHUNK;

    while (amountToProcess > 0) 
    {
        if (index_tail >= numCells)
        {
            break;
        }

        vec2uint nextToProcess = indexQueue[index_tail];
        index_tail++;        
        process_cell(nextToProcess.i, nextToProcess.j, chunk);      
    }  

    for (int i = 0; i < CELLS_PER_CHUNK; i++) 
    {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) 
        {
     
            pathfinding_data & current = pf_data[i][j]; 
            current.moveDir = {0,0};
            gridCell &cell = chunk->cells[i][j];

            if (IS_CELL_FREE(cell)) 
            { 
                if (current.distanceToTarget > -1) 
                {
                    int minDist = 999999;

                    for (int offsetIndex = 0; offsetIndex < 8; offsetIndex++) 
                    {     
                        vec2int &offset = offsets[offsetIndex];
                        vec2int coordsToCheck = {i + offset.x, j + offset.y};
                        
                        if (INDEX_IN_BOUNDS(coordsToCheck))
                        {          
                            pathfinding_data & dataToCheck = pf_data[coordsToCheck.x][ coordsToCheck.y];
                            gridCell & cellToCheck = chunk->cells[coordsToCheck.x][coordsToCheck.y];
                            
                            if (IS_CELL_FREE(cellToCheck)) 
                            {
                                if (dataToCheck.distanceToTarget > -1 && dataToCheck.distanceToTarget < minDist) 
                                {
                                    minDist = dataToCheck.distanceToTarget;
                                    current.moveDir = offset;
                                    cell.directionIndex = offsetIndex;
                                }    
                            }
                        }
                    }        
                }
            }
        }
    }
}
