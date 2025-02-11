#include "g_main.hpp"
#include "scratch.hpp"
#include "shader_store.hpp"
#include "particle_system.hpp"
#include "include/rlgl.h"
#include "game_src/state_main.hpp"

#ifdef _USE_EDITOR
#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"
#endif

#define MINIMUM(a, b) a < b ? a : b

RenderTexture2D renderTarget;
void render_frame( GameState *currentState, GameContext & gameContext) ;

RenderTexture2D LoadRenderTextureDepthTex(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth texture buffer (instead of raylib default renderbuffer)
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}


// Unload render texture from GPU memory (VRAM)
void UnloadRenderTextureDepthTex(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // Color texture attached to FBO is deleted
        rlUnloadTexture(target.texture.id);
        rlUnloadTexture(target.depth.id);

        // NOTE: Depth texture is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}


void InitializeGame(GameContext & context) 
{ 
    // initialize the game context
    context.screenWidth = GetScreenWidth();
    context.screenHeight = GetScreenHeight();
    context.renderWidth = context.screenWidth / 2;
    context.renderHeight = context.screenHeight / 2; 
    context.applicationTime = GetTime();
    context.deltaTime = GetFrameTime();
    context.running = true;

    ParticleSystemInit();
    InitAudioDevice();
    ShaderStore_Init();
 
    renderTarget = LoadRenderTextureDepthTex(context.renderWidth, context.renderHeight);    
    SetTextureFilter(renderTarget.texture, TEXTURE_FILTER_BILINEAR);
 
    register_state(E_GameState::GS_MAIN, &GET_STATE(MAIN));
    set_initial_game_state(E_GameState::GS_MAIN, context);        
}

void OnWindowResize(GameContext & context) 
{ 
    context.screenWidth = GetScreenWidth();
    context.screenHeight = GetScreenHeight();
    context.renderWidth = context.screenWidth / 2;
    context.renderHeight = context.screenHeight / 2;
    
    UnloadRenderTexture(renderTarget);
    renderTarget = LoadRenderTextureDepthTex(context.renderWidth, context.renderHeight);
    SetTextureFilter(renderTarget.texture, TEXTURE_FILTER_BILINEAR); 
}

void ProcessInput(GameContext & gameContext) 
{
    gameContext.MousePosition = GetMousePosition();
    gameContext.MouseDelta = GetMouseDelta();
    gameContext.MouseScroll = GetMouseWheelMoveV();     
    gameContext.applicationTime = GetTime();
    gameContext.deltaTime = GetFrameTime();
}

void ProcessFrame(GameContext & gameContext) 
{
    ProcessInput(gameContext);    
    
    GameState *currentState = get_current_state();
    currentState->OnUpdate(gameContext);  

    render_frame(currentState, gameContext);
    check_for_state_change(gameContext);
}

void DestroyGame(GameContext & context) 
{    
    GameState *currentState = get_current_state();
    currentState->OnStateExit(context);
    ShaderStore_Release();
    CloseAudioDevice();        
    UnloadRenderTextureDepthTex(renderTarget);
}

Rectangle sourceRect, dstRect;
Color clearColor = {15, 15, 15, 255};

void updateBlitRects(GameContext & context) 
{ 
    float scale = MINIMUM( float(context.screenWidth) / float(context.renderWidth), float(context.screenHeight) / float (context.renderHeight));  
    float rWidth = (float) renderTarget.texture.width;
    float rHeight = (float) renderTarget.texture.height;

    sourceRect = 
    { 
        0.0f, 0.0f, 
        rWidth, -rHeight 
    };

    float scaledWidth =  (float(context.renderWidth)    * scale);
    float scaledHeight =  (float(context.renderHeight)  * scale);
    
    dstRect = 
    { 
        (context.screenWidth - scaledWidth) * 0.5f, 
        (context.screenHeight - scaledHeight) * 0.5f,
        scaledWidth, 
        scaledHeight
    };
}

void render_frame( GameState *currentState, GameContext & gameContext) 
{ 
    if (GetScreenWidth() != gameContext.screenWidth || GetScreenHeight() != gameContext.screenHeight) 
    { 
        OnWindowResize(gameContext);
    }

    { ScopedRenderTextureBlock setTarget(renderTarget);
        
        ClearBackground(clearColor);          
        currentState->OnRender(gameContext);           
    }


    { ScopedDrawBlock drawScope;

        ClearBackground(BLACK);    
        updateBlitRects(gameContext);
 
        
        Shader & bloom = get_postprocessor(PostProcessTypes::FX_BLOOM);
        float quality = 1.0f;
        float samples = 5.0f;

        SetShaderValue(bloom, GetShaderLocation(bloom, "quality"), &quality, SHADER_UNIFORM_FLOAT);
        SetShaderValue(bloom, GetShaderLocation(bloom, "samples"), &samples, SHADER_UNIFORM_FLOAT);


        {
            ScopedShaderBlock useShader(bloom);
            DrawTexturePro(renderTarget.texture, sourceRect, dstRect, { 0, 0 }, 0.0f, WHITE);    
        }
    }
}