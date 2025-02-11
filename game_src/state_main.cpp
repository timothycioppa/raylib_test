#include <stdio.h>
#include <math.h>
#include <string.h>
#include "state_main.hpp"
#include "../scratch.hpp"
#include "../include/raymath.h"
#include "../gameplay.hpp"
#include "../camera.hpp"
#include "../shader_store.hpp"
#include "../include/rlgl.h"
#include "../particle_system.hpp"
#include "../mesh_utils.hpp"
#include "tile_map.hpp"
#include "../raylib_extensions.hpp"
#include "pathfinding.hpp"
#include "../AABB.hpp"

#define floatArray3(v) { v.x, v.y, v.z }
#define CAMERA_MOVE_SPEED  5.0f
#define CAMERA_MOUSE_MOVE_SENSITIVITY  0.1f
#define MAX_ENTITIES 32
#define NUM_BODIES 10
#define MAXIMUM(a,b) a > b ? a : b

Vector3 groundPlaneCursorPosition = Vector3Zero();

struct PlaneTransform 
{
    float roll;
    float pitch;
    float yaw;
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};

struct entity 
{ 
    float moveSpeed;
    BoundingBox boundingBox;
    Vector3 position;
    Vector2 moveDir;    
    bool selected;
};

bool RaycastToGroundPlane(Camera & cam, Vector2 screenPosition, Vector3 *hitPosition) ;
void update_transform (PlaneTransform & transform, Model model) ;

void RenderOverlay(GameContext & context);
void update_camera(GameContext &context);
Vector3 random_vector(int, int);
gridCell *getCell(Vector3 position, gridChunk * chunk);
void render_chunk (gridChunk & chunk);
float lerpSpeed = 0.01f;

camera mainCamera;
Light lights[MAX_LIGHTS] = { 0 };
Vector3 initialVelocity = {0,2,0};
Vector3 initialPosition = {0,1,0};
BoundingBox groundBox;
bool jumping = false;
Vector3 bbExtents = {0.2f, 0.5f, 0.2f}; 
float jumpTimer = 0.0f;
float defaultHorizontal = 90.0f;
float defaultVertical = -70.0f;
bool dragging = false;
Vector2 startDragPosition;
Vector3 startDragPositionWorld;
Vector3 pos = {0,1,0};
BoundingBox dragBox;
entity entities[MAX_ENTITIES];
int numEntities = 0;

entity player;
ParticleSystem particleSystem;
Mesh particleMesh;
Material particleMaterial;
Texture2D particleTex;

bool waitingForRightClick;
float rightClickTimer = 0.0f;
float rightClickDelay = 0.15f;
Music backgroundMusic;

Vector3 center = {50, 20, 50};
Vector3 center2 = {-50, 20, -50};
float mass = 1000.0f;
float mass2 = 1000.0f;

Vector3 quadPoints[4] = 
{
    {-1000,  0, -1000}, 
    {-1000,  0,  1000}, 
    { 1000,  0,  1000}, 
    { 1000,  0, -1000}
};

bool moveRequested = false;
Vector3 moveTarget, initialPlayerPosition;
float playerMoveSpeed = 15.0f;
float planeAcceleration = 20.0f;

Vector3 cameraOffset = {0, 10, -10};
tile_map tileMap;
Vector2 hoveredCellIndex;
bool hoveredOverCell = false;
BoundingBox playerBB;

Vector2 playerMoveDir = {0,0};

void randomize_chunk(gridChunk & chunk) 
{
    for (int i = 0; i < CELLS_PER_CHUNK; i++) {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) {
           if (GetRandomValue(0, 100) < 10) { 
                chunk.cells[i][j].cellFlags = 0x01;
           } 
        }
    }
}

Texture2D arrows[8];
Material arrowMat;
Mesh arrowMesh;

STATE_ENTER(MAIN)
{
    init_tilemap(tileMap);
    initializeChunk(0, 0, tileMap); 
    foreach_active_chunk(randomize_chunk, tileMap);

    AABB_Update({0,5,0}, {1,5,1}, playerBB);

    backgroundMusic = LoadMusicStream("resources/audio/hanger.wav");
    PlayMusicStream(backgroundMusic);

    Shader & lit = get_shader(ShaderType::SHADER_LIT);
    float ambient[4] = { 1,1,1, 1.0f };
    float fogDensity = 0.01f;
    SetShaderValue(lit, GetShaderLocation(lit, "ambient"), ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(lit, GetShaderLocation(lit, "fogDensity"), &fogDensity, SHADER_UNIFORM_FLOAT);

    lights[0] = CreateLight(LIGHT_DIRECTIONAL, {0,50,0}, Vector3Zero(), WHITE, lit);
    lights[1] = CreateLight(LIGHT_DIRECTIONAL, {0,50,0}, Vector3Zero(), WHITE, lit);
    lights[2] = CreateLight(LIGHT_DIRECTIONAL, {0,50,0}, Vector3Zero(), WHITE, lit);
    lights[3] = CreateLight(LIGHT_DIRECTIONAL, {0,50,0}, Vector3Zero(), WHITE, lit);

    lights[0].enabled = true;
    lights[1].enabled = false;
    lights[2].enabled = false;
    lights[3].enabled = false;
    

    float size =  float(CELL_WIDTH);
    arrowMesh = GenMeshPlane(size, size, 1, 1);
    arrowMat = LoadMaterialDefault();    
    arrowMat.shader = lit;
    char* buff = (char*) scratch_alloc(256);

    for (int i = 0; i < 8; i++) 
    {
        sprintf(buff, "resources/textures/directions/direction(%d).png", i);
        arrows[i] = LoadTexture(buff);
    }

    SetMaterialTexture(&arrowMat, MATERIAL_MAP_ALBEDO, arrows[0]);    

    for (int i = 0; i < 5; i++) 
    {
        entity & e = entities[i];
        e.moveDir = {0,0};
        e.moveSpeed = float(GetRandomValue(10, 15));
        e.position = {4.0f * i, 1, 4.0f * i};
        e.boundingBox.min = e.position - Vector3One();
        e.boundingBox.max = e.position + Vector3One();
        e.selected = false;
    }

    numEntities = 5;
    
    player.position = {-15, 0.5f, -15};
    player.selected = false;
    AABB_Update({-15, 0.5f, -15}, {0.5, 0.5, 0.5}, player.boundingBox);


    mainCamera.cam = &context.camera;
    mainCamera.cam->position = cameraOffset;
    mainCamera.Horizontal = defaultHorizontal;
    mainCamera.Vertical = defaultVertical;
    mainCamera.cam->fovy = 45.0f;
    mainCamera.cam->projection = CAMERA_PERSPECTIVE;
    rotate_camera(mainCamera);

    float chunkSize = float(CELL_WIDTH) * float(CELLS_PER_CHUNK);

    AABB_Update(
        {0.5f * chunkSize, -0.5f, 0.5f * chunkSize},
        {0.5f * chunkSize, 0.5f, 0.5f * chunkSize},
        groundBox
    );   
}


STATE_UPDATE(MAIN)
{ 
   

    for (int i = 0; i < 5; i++) 
    {
        entity & e = entities[i];
        Vector3 center = AABB_Center(e.boundingBox);
        Vector3 size = AABB_Size(e.boundingBox);  
        pathfinding_data *pfd = getPathfindingData(center);

        if (pfd)
        {    
            e.moveDir = Vector2Lerp(e.moveDir, {float(pfd->moveDir.x), float(pfd->moveDir.y)}, lerpSpeed);
            e.moveDir = Vector2Normalize(e.moveDir);

            center += 
            {
                e.moveDir.x * context.deltaTime * e.moveSpeed, 
                0.0f, 
                e.moveDir.y * context.deltaTime * e.moveSpeed, 
            };

            gridChunk *chunk = getChunk(0, 0, tileMap);
            gridCell* cell = getCell(center, chunk);
            
            if (cell) 
            {
                if (IS_CELL_FREE(*cell)) 
                {
                    AABB_Update(center, size * 0.5f, e.boundingBox);
                }
            }
            
        }    
    }

    Vector3 center = AABB_Center(playerBB);  
    pathfinding_data *pfd = getPathfindingData(center);

    if (pfd)
    {

        playerMoveDir = Vector2Lerp(playerMoveDir, {float(pfd->moveDir.x), float(pfd->moveDir.y)}, lerpSpeed);

        center += 
        {
            playerMoveDir.x * context.deltaTime * playerMoveSpeed, 
            0.0f, 
            playerMoveDir.y * context.deltaTime * playerMoveSpeed, 
        };

        gridChunk *chunk = getChunk(center,tileMap);
        gridCell* cell = getCell(center, chunk);

        if (cell) 
        {
            if (IS_CELL_FREE(*cell)) 
            {
                AABB_Update(center, {1,5,1}, playerBB);
            }
        }
        
    }

    groundPlaneCursorPosition = Vector3Zero();
    bool cursorOverGroundPlane = RaycastToGroundPlane(*mainCamera.cam, context.MousePosition, &groundPlaneCursorPosition);
    bool overCell = false;
    
    if (cursorOverGroundPlane) 
    {

        float chunkSize = CELL_WIDTH * float(CELLS_PER_CHUNK);
        float x = groundPlaneCursorPosition.x;
        float y = groundPlaneCursorPosition.z;

        if (x > 0 && x < chunkSize && y > 0 && y < chunkSize)
        {
            int i = int(x) / int(CELL_WIDTH);
            int j = int(y) / int(CELL_WIDTH);
            hoveredCellIndex = {float(i), float(j)};
            overCell = true; 
        }
    }

    hoveredOverCell = overCell;

    if (hoveredOverCell) 
    {
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) 
        {
            gridChunk *chunk = getChunk(groundPlaneCursorPosition, tileMap);
            if (chunk)
            {
                run_pathfinder(hoveredCellIndex.x, hoveredCellIndex.y, chunk);
            }
        }
    }

    if (moveRequested) 
    {
        Vector3 moveDirection = Vector3Normalize(moveTarget - initialPlayerPosition);
        moveDirection = moveDirection * context.deltaTime * playerMoveSpeed;
        player.position = player.position + moveDirection; 
        AABB_Update(player.position,{0.5, 0.5, 0.5}, player.boundingBox);
    }

    if (waitingForRightClick) 
    {
        rightClickTimer += context.deltaTime;
        if (rightClickTimer > rightClickDelay) 
        {
            waitingForRightClick = false;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) 
    {
        waitingForRightClick = true;
        rightClickTimer = 0.0f;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) 
    { 
        if (waitingForRightClick) 
        {
            waitingForRightClick = false;

            if (cursorOverGroundPlane) 
            {
                moveTarget = groundPlaneCursorPosition;
                moveRequested = true;
                initialPlayerPosition = player.position;
                moveTarget.y = 0.5f;
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if(cursorOverGroundPlane) 
        {
            startDragPositionWorld = groundPlaneCursorPosition;
            startDragPosition = context.MousePosition;
            dragging = true;
        }
    }

    if (dragging) 
    {
        Vector3 currentGroundPosition = Vector3Zero();

        if (cursorOverGroundPlane) 
        {
            currentGroundPosition = groundPlaneCursorPosition;
            Vector2 dimensions = 
            {
                abs(currentGroundPosition.x - startDragPositionWorld.x),
                abs(currentGroundPosition.z - startDragPositionWorld.z)
            } ;

            #define MINIMUM(a,b) (a < b ? a : b)    
            Vector2 minPoint = 
            {
                MINIMUM(currentGroundPosition.x, startDragPositionWorld.x),
                MINIMUM(currentGroundPosition.z, startDragPositionWorld.z)
            };
            #undef min

            dragBox.min = {minPoint.x, 0.1f, minPoint.y};
            dragBox.max = {minPoint.x + dimensions.x, 6.0f, minPoint.y + dimensions.y};
        } 
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        dragging = false;     
    }

    update_camera(context);
    UpdateMusicStream(backgroundMusic);
}

STATE_RENDER(MAIN)
{   
    { Scoped3DBlock block(context.camera);
    
        foreach_active_chunk(render_chunk, tileMap);

        for (int i = 0; i < 5; i++) 
        {
            entity & e = entities[i];
            DrawBoundingBox(e.boundingBox, WHITE);
        }

        if (hoveredOverCell)
        {
            int i = int(hoveredCellIndex.x);
            int j = int (hoveredCellIndex.y);
            gridChunk *pchunk = getChunk(groundPlaneCursorPosition, tileMap);
            gridCell & cell = pchunk->cells[i][j];
            Vector3 cellCenter = cellPositionInChunk(i,j, *pchunk);
            
            DrawCubeWiresV(
                cellCenter,
                {CELL_WIDTH, CELL_WIDTH, CELL_WIDTH},
                GREEN
            );
        }

        DrawBoundingBox(playerBB, BLUE);

        // draw the ground box
        DrawCubeWiresV(
            AABB_Center(groundBox),
            AABB_Size(groundBox), 
            WHITE
        );

        // Draw the selection region we're currently dragging
        if (dragging) 
        {
            DrawCubeWiresV(
                AABB_Center(dragBox),
                AABB_Size(dragBox), 
                GREEN
            );
        }      
    } 
    
    RenderOverlay(context);  
}

STATE_EXIT(MAIN)
{
    UnloadMusicStream(backgroundMusic);
} 

void update_camera(GameContext & context)
{ 
    // look around while holding the middle mouse button
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) 
    {
        Vector2 mousePositionDelta = context.MouseDelta;
        mainCamera.Horizontal +=mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY;
        mainCamera.Vertical -= mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY;
        rotate_camera(mainCamera);
    }

    // zoom in and out with the scroll wheel
    Vector2 scroll = context.MouseScroll;
    float scrollSpeed = 150.0f;

    if(scroll.y != 0) 
    {
        cameraOffset.y -= scroll.y * context.deltaTime * scrollSpeed;
        rotate_camera(mainCamera);
    }

    if (IsKeyDown(KEY_S)) 
    {
        cameraOffset.x += playerMoveSpeed * context.deltaTime;
    }

    if (IsKeyDown(KEY_F)) 
    {
        cameraOffset.x -= playerMoveSpeed * context.deltaTime;
    }

    if (IsKeyDown(KEY_D)) 
    {
        cameraOffset.z -= playerMoveSpeed * context.deltaTime;
    }

    if (IsKeyDown(KEY_E)) 
    {
        cameraOffset.z += playerMoveSpeed * context.deltaTime;
    }


    mainCamera.cam->position =  cameraOffset;
    mainCamera.cam->target = mainCamera.cam->position + mainCamera.forward;
}

void RenderOverlay(GameContext & context) 
{  
    float rWidth = context.renderWidth;
    float rHeight = context.renderHeight;
    float height = 100;
    float top = rHeight - height;

    DrawRectangle(0, top, rWidth, height, BLACK );

    Rectangle rect = {20, top + 20, 200, 60};
    DrawRectangleRoundedLinesEx(rect, 0.2f, 5, 1, WHITE);
}



Vector3 random_vector(int min, int max) {
    return 
    {
        (float)GetRandomValue(min, max), 
        (float)GetRandomValue(min, max), 
        (float)GetRandomValue(min, max)
    };
}

bool RaycastToGroundPlane(Camera & cam, Vector2 screenPosition, Vector3 *hitPosition) 
{
    Ray ray = GetScreenToWorldRay(screenPosition, cam);            
    RayCollision collsion = GetRayCollisionQuad(ray, quadPoints[0], quadPoints[1], quadPoints[2], quadPoints[3]);

    if (collsion.hit) 
    {
        *hitPosition = collsion.point;
        return true;
    }   

    return false;
}

void update_transform (PlaneTransform & transform, Model model) 
{
    Vector3 angle = 
    { 
        DEG2RAD * transform.pitch, 
        DEG2RAD * transform.yaw, 
        DEG2RAD * transform.roll 
    };

    model.transform = MatrixRotateXYZ(angle);
    
    transform.forward = TransformDirection(model.transform, {0,0,1});
    transform.right = TransformDirection(model.transform, {1, 0, 0});
    transform.up = TransformDirection(model.transform, {0,1,0});
}


gridCell *getCell(Vector3 position, gridChunk * chunk) 
{ 
    float cellSize = CELL_WIDTH;
    float chunkSize = cellSize * float(CELLS_PER_CHUNK); 
    gridCell *result = 0;
   
    if (position.x >= 0 && position.x <= chunkSize && position.z >= 0 && position.z < chunkSize)
    {
        int playerI = int(position.x) / int(CELL_WIDTH);
        int playerJ = int(position.z) / int(CELL_WIDTH);
        result =  &chunk->cells[playerI][playerJ];
    }

    return result; 
}


void render_chunk (gridChunk & chunk) 
{ 
    Vector3 size = chunkDimensions(chunk);
    Vector3 center = chunkCenter(chunk);
    DrawGrid(CELLS_PER_CHUNK, CELL_WIDTH, center); 
 
    Vector3 offset = {0, 0.5f * CELL_WIDTH * CELLS_PER_CHUNK, 0};
    DrawCubeWires(center + offset, size.x, size.y, size.z, {74, 74, 0, 255});

    for (int i = 0; i < CELLS_PER_CHUNK; i++) 
    {
        for (int j = 0; j < CELLS_PER_CHUNK; j++) 
        {
            gridCell & cell = chunk.cells[i][j];
            Vector3 cellCenter = cellPositionInChunk(i,j, chunk);

            if (!IS_CELL_FREE(cell))
            {
                Shader & lit = get_shader(ShaderType::SHADER_LIT);
               

                { ScopedShaderBlock block(lit);
                  
                    lights[0].position = mainCamera.cam->position;
                    lights[0].target = mainCamera.cam->target;
                    Vector3 pos = mainCamera.cam->position;

                    setCameraPosition(lit, mainCamera.cam->position);
                    UpdateLightValues(lit, lights[0]);
    
                    DrawCube(
                        cellCenter,
                        CELL_WIDTH, CELL_WIDTH, CELL_WIDTH,
                        GREEN
                    ); 
                }

            } 
            // else 
            // {
            //     SetMaterialTexture(&arrowMat, MATERIAL_MAP_ALBEDO, arrows[cell.directionIndex]);
            //     DrawMesh(arrowMesh, arrowMat, MatrixTranslate(cellCenter.x, cellCenter.y, cellCenter.z)); 
            // }
        }
    }
}

EXPORT_STATE(MAIN);