#include "state_main.hpp"
#include "scratch.hpp"
#include <stdio.h>
#include <math.h>
#include "include/raymath.h"
#include "gameplay.hpp"
#include "camera.hpp"
#include "shader_store.hpp"
#include "include/rlgl.h"
#include "particle_system.hpp"
#include <string.h>
#include "mesh_utils.hpp"

#define floatArray3(v) { v.x, v.y, v.z }
#define CAMERA_MOVE_SPEED  5.0f
#define CAMERA_MOUSE_MOVE_SENSITIVITY  0.1f

Model planeModel;
float planeSpeed = 0.0f;
Vector3 camOffsetFromPlane = {0, 10, -5};

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



void update_transform (PlaneTransform & transform) 
{
    Vector3 angle = 
    { 
        DEG2RAD * transform.pitch, 
        DEG2RAD * transform.yaw, 
        DEG2RAD * transform.roll 
    };

    planeModel.transform = MatrixRotateXYZ(angle);


    transform.forward = TransformDirection(planeModel.transform, {0,0,1});
    transform.right = TransformDirection(planeModel.transform, {1, 0, 0});
    transform.up = TransformDirection(planeModel.transform, {0,1,0});


}

PlaneTransform planeTransform;
Vector3 planeVelocity;

void RenderOverlay(GameContext & context);
void update_AABB(Vector3 center, Vector3 extents, BoundingBox & bb ) ;
void update_camera(GameContext &context);
Vector3 get_force(Vector3 position) ;
void simulate_system(float dt) ;
Vector3 random_vector(int, int);
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

struct entity 
{ 
    BoundingBox boundingBox;
    Vector3 position;    
    bool selected;
};

#define MAX_ENTITIES 32
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
float playerMoveSpeed = 5.0f;
float planeAcceleration = 20.0f;

#define NUM_BODIES 10
struct Body 
{ 
    Vector3 center;
    float mass;
    Vector3 velocity;
};

Body bodies[NUM_BODIES] = 
{
    {
        {150, 25, 150}, 
        10400.0f,
        Vector3Zero()
    },

    {
        {120, 50, 150}, 
        1000.0f,
        Vector3Zero()
    },

    {
        {150, 25, 150}, 
        1000.0f,
        Vector3Zero()
    },

    {
        {-150, 50, -150}, 
        14200.0f,
        Vector3Zero()
    },

        {
        {-150, 25, 150}, 
        1000.0f,
        Vector3Zero()
    },

     {
        {150, 300, 150}, 
        10400.0f,
        Vector3Zero()
    },

    {
        {120, 300, 150}, 
        1000.0f,
        Vector3Zero()
    },

    {
        {150, 300, 150}, 
        1000.0f,
        Vector3Zero()
    },

    {
        {-150, 300, -150}, 
        14200.0f,
        Vector3Zero()
    },

        {
        {-150, 300, 150}, 
        1000.0f,
        Vector3Zero()
    },
};

#define MAXIMUM(a,b) a > b ? a : b

STATE_ENTER(MAIN)
{      

    planeTransform.position = {0,0,0};
    planeTransform.roll = 0.0f;
    planeTransform.pitch = 0.0f;
    planeTransform.yaw = 0.0f;
    update_transform(planeTransform);
    planeSpeed = 0.0f;
    planeVelocity = {0,0,0};
    planeModel = LoadModel("resources/models/plane.obj");
    
    backgroundMusic = LoadMusicStream("resources/audio/hanger.wav");
    PlayMusicStream(backgroundMusic);

    Shader & litShader = get_shader(ShaderType::SHADER_LIT_INSTANCED);
    float ambient[4] = { 1,1,1, 1.0f };
    float fogDensity = 0.02f;
    SetShaderValue(litShader, GetShaderLocation(litShader, "ambient"), ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(litShader, GetShaderLocation(litShader, "fogDensity"), &fogDensity, SHADER_UNIFORM_FLOAT);

    particleMesh = load_mesh("resources/meshes/cube");
    particleMaterial = LoadMaterialDefault();
    particleMaterial.shader = litShader;
    particleTex = LoadTexture("resources/textures/0001.png");
    SetMaterialTexture(&particleMaterial, MATERIAL_MAP_ALBEDO, particleTex);        
  
    if (AllocateSystem(250, particleSystem)) 
    { 
        for (int i = 0; i < particleSystem.particleCount; i++) 
        {
            particleSystem.positions[i] = random_vector(-50, 50);    
            particleSystem.velocities[i] = random_vector(-1, 1);
            particleSystem.matrices[i] = MatrixIdentity();
            particleSystem.ageLifetimes[i].x = 0.0f;
            particleSystem.ageLifetimes[i].y = 1.0f;
        }
    }
  
    for (int i = 0; i < 5; i++) 
    {
        entity & e = entities[i];
        e.position = {4.0f * i, 1, 4.0f * i};
        e.boundingBox.min = e.position - Vector3One();
        e.boundingBox.max = e.position + Vector3One();
        e.selected = false;
    }

    numEntities = 5;
    player.position = {-15, 0.5f, -15};
    player.selected = false;
    update_AABB({-15, 0.5f, -15}, {0.5, 0.5, 0.5}, player.boundingBox);

    mainCamera.cam = &context.camera;
    mainCamera.cam->position = planeTransform.position + camOffsetFromPlane;
    mainCamera.Horizontal = defaultHorizontal;
    mainCamera.Vertical = defaultVertical;
    mainCamera.cam->fovy = 90.0f;
    mainCamera.cam->projection = CAMERA_PERSPECTIVE;
    rotate_camera(mainCamera);

    update_AABB(
        {0, -0.5f, 0},
        {100, 0.5f, 100},
        groundBox
    );

    lights[0] = CreateLight(LIGHT_POINT, { 0,15,0 }, Vector3Zero(), {40, 40, 40, 255}, litShader);
    lights[1] = CreateLight(LIGHT_POINT,{ 2, 1, 2 }, Vector3Zero(), RED, litShader);
    lights[2] = CreateLight(LIGHT_POINT, { -2, 1, 2 }, Vector3Zero(), GREEN, litShader);
    lights[3] = CreateLight(LIGHT_POINT, { 2, 1, -2 }, Vector3Zero(), BLUE, litShader);

    lights[0].enabled = true;
    lights[1].enabled = false;
    lights[2].enabled = false;
    lights[3].enabled = false;

    UpdateLightValues(litShader, lights[0]);
    UpdateLightValues(litShader, lights[1]);
    UpdateLightValues(litShader, lights[2]);
    UpdateLightValues(litShader, lights[3]);
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

STATE_UPDATE(MAIN)
{ 
    UpdateMusicStream(backgroundMusic);   // Update music buffer with new stream data
    Vector3 groundPlaneCursorPosition = Vector3Zero();
    bool cursorOverGroundPlane = RaycastToGroundPlane(*mainCamera.cam, context.MousePosition, &groundPlaneCursorPosition);

    if (moveRequested) 
    {
        Vector3 moveDirection = Vector3Normalize(moveTarget - initialPlayerPosition);
        moveDirection = moveDirection * context.deltaTime * playerMoveSpeed;
        player.position = player.position + moveDirection; 
        update_AABB(player.position,{0.5, 0.5, 0.5}, player.boundingBox);
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
            dragBox.max = {minPoint.x + dimensions.x, 3.0f, minPoint.y + dimensions.y};
        } 
    }

    float rotationSpeed = 15.0f;

    if (IsKeyDown(KEY_E)) 
    {
        planeTransform.pitch += context.deltaTime * rotationSpeed;
    }

    if (IsKeyDown(KEY_D)) 
    {
        planeTransform.pitch -= context.deltaTime * rotationSpeed;
    }

    if (IsKeyDown(KEY_S)) 
    {
        planeTransform.yaw += context.deltaTime * rotationSpeed;
    }
 
    if (IsKeyDown(KEY_F)) 
    {
        planeTransform.yaw -= context.deltaTime * rotationSpeed;
    }

    if (IsKeyDown(KEY_W)) 
    {
        planeTransform.roll += context.deltaTime * rotationSpeed;
    }
 
    if (IsKeyDown(KEY_R)) 
    {
        planeTransform.roll -= context.deltaTime * rotationSpeed;
    }


    if (IsKeyDown(KEY_SPACE)) 
    {
        Vector3 acceleration = planeTransform.forward * planeAcceleration;
        planeVelocity += (acceleration * context.deltaTime);
    }


    Vector3 gravity = { 0, 1.0f, 0 };
    //planeVelocity -= gravity * context.deltaTime;

    planeTransform.position += (planeVelocity * context.deltaTime);
    
    update_transform(planeTransform);

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        dragging = false;     
    }

    update_camera(context);
    simulate_system(context.deltaTime);

    simulation_context simcontext = {
        context.deltaTime,
        mainCamera.cam->position,
        get_force
    };

    SimulateSystem(particleSystem, simcontext);
}

Vector3 AABB_Center(BoundingBox &box) {
    return {
        box.min.x + 0.5f * (box.max.x - box.min.x),
        box.min.y + 0.5f * (box.max.y - box.min.y),
        box.min.z + 0.5f * (box.max.z - box.min.z)
    };
}

Vector3 AABB_Size(BoundingBox &box){
    return {
        box.max.x - box.min.x,
        box.max.y - box.min.y,
        box.max.z - box.min.z
    };
}

STATE_RENDER(MAIN)
{   
    BeginMode3D(context.camera);
    {
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
                WHITE
            );
        }

        // draw all celestial bodies
        for (int i = 0; i < NUM_BODIES; i++ )
        {
            DrawSphereWires(bodies[i].center, 25.0f, 8, 8, WHITE);
        }

        // draw plane (the flight thing, not the ground plane)
        DrawModelWires(planeModel, planeTransform.position, .1, WHITE);
      
        {
            float cameraPos[3] = {mainCamera.cam->position.x, mainCamera.cam->position.y, mainCamera.cam->position.z};
            SetShaderValue(particleMaterial.shader, particleMaterial.shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
            DrawMeshInstanced(particleMesh, particleMaterial, particleSystem.matrices, particleSystem.particleCount);
         }

         EndMode3D();     
    } 
    

    char* buff = (char*) scratch_alloc(128);
    
    sprintf(buff, "speed: %.2g\n", planeSpeed);
    DrawText(buff, 20, 60, 15.0f, WHITE);
    
    sprintf(buff, "pitch: %.2g yaw: %.2g roll: %.2g\n", planeTransform.pitch, planeTransform.yaw, planeTransform.roll);
    DrawText(buff, 20, 80, 15.0f, WHITE);

    RenderOverlay(context);  
}

STATE_EXIT(MAIN)
{
    UnloadMusicStream(backgroundMusic);   // Unload music stream buffers from RAM
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
        camOffsetFromPlane.y -= scroll.y * context.deltaTime * scrollSpeed;
        rotate_camera(mainCamera);
    }

    mainCamera.cam->position = planeTransform.position + camOffsetFromPlane;
    mainCamera.cam->target = mainCamera.cam->position + mainCamera.forward;

}

void RenderOverlay(GameContext & context) 
{  
    float rWidth = context.renderWidth;
    float rHeight = context.renderHeight;

    DrawRectangle(0, rHeight - 50, rWidth, 50, BLACK );
    int selectedCount = 0;
    float gap = 10;
    int fullWidth = 50;
    
    for (int i = 0; i < numEntities; i++) 
    {
        Color color = Color { 255,255, 255, 255};
        if (entities[i].selected) 
        {
            int pX = gap + fullWidth * selectedCount;
            int pY = rHeight - 50 + gap;
            DrawRectangleLines(pX, pY, 30, 30, WHITE);
            selectedCount++;
        }
    }

    if (player.selected) 
    {
         int pX = gap + fullWidth * selectedCount;
        int pY = rHeight - 50 + gap;
        DrawRectangleLines(pX, pY, 30, 30, GREEN);
    }
}

void update_AABB(Vector3 center, Vector3 extents, BoundingBox & bb ) 
{
    bb.min = {center.x - extents.x, center.y - extents.y, center.z - extents.z} ;
    bb.max = {center.x + extents.x, center.y + extents.y, center.z + extents.z} ;
}

Vector3 get_force(Vector3 position) 
{
    Vector3 result = {0,0,0};
    
    for (int i = 0; i < NUM_BODIES; i++) 
    {
        float mass = 10.0f;
        Vector3 diff = bodies[i].center - position;
        Vector3 direction = Vector3Normalize(diff);
        float sqDist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        sqDist = MAXIMUM(sqDist, 100.0f);
        result += (direction * mass * bodies[i].mass / sqDist);
    }

    return result;
}

void simulate_system(float dt) 
{ 
    for (int i = 0; i < NUM_BODIES; i++) 
    {
        Body & body = bodies[i];
        float m1 = body.mass / 50.0f;
        Vector3 f = {0,0,0};

        for (int j = 0; j < NUM_BODIES; j++) 
        {
            if (i != j) 
            {
                Body & other = bodies[j];
                float m2 = other.mass / 50.0f;
                Vector3 diff = other.center - body.center;
                float sqDist = Vector3DotProduct(diff, diff);
                sqDist = MAXIMUM(sqDist, 1.0f);
                f += Vector3Normalize(diff) * ((m1 * m2) / sqDist);
            }
        }

        body.velocity += (f * dt);
    }

    for (int i = 0; i < NUM_BODIES; i++) 
    {
        Body & body = bodies[i];
        body.center += (body.velocity * dt);
    }
}

Vector3 random_vector(int min, int max) {
    return 
    {
        (float)GetRandomValue(min, max), 
        (float)GetRandomValue(min, max), 
        (float)GetRandomValue(min, max)
    };
}

EXPORT_STATE(MAIN);