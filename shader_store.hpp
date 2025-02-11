#ifndef _SHADER_STORE_HPP
#define _SHADER_STORE_HPP

#include "include/raylib.h"

class ScopedShaderBlock 
{ 
    public:
        ScopedShaderBlock(Shader & s) 
        { 
            BeginShaderMode(s);
        }
        
        ~ScopedShaderBlock() { 
            EndShaderMode();
        }
};

class Scoped3DBlock 
{ 
    public:
        Scoped3DBlock(Camera & c) 
        { 
            BeginMode3D(c); 
        }
        
        ~Scoped3DBlock() { 
            EndMode3D();
        }
};

class ScopedDrawBlock 
{ 
    public:
        ScopedDrawBlock() 
        { 
            BeginDrawing();
        }
        
        ~ScopedDrawBlock() {
            EndDrawing();
        }
};

class ScopedRenderTextureBlock 
{ 
    public:
        ScopedRenderTextureBlock(RenderTexture2D & tex) 
        { 
            BeginTextureMode(tex);
        }
        
        ~ScopedRenderTextureBlock() {
            EndTextureMode();
        }
};

enum ShaderType 
{ 
    SHADER_LIT = 0,
    SHADER_LIT_INSTANCED = 1,
    SHADER_PBR = 2,
    SHADER_COUNT = 3
};


typedef enum {
    FX_GRAYSCALE = 0,
    FX_POSTERIZATION,
    FX_DREAM_VISION,
    FX_PIXELIZER,
    FX_CROSS_HATCHING,
    FX_CROSS_STITCHING,
    FX_PREDATOR_VIEW,
    FX_SCANLINES,
    FX_FISHEYE,
    FX_SOBEL,
    FX_BLOOM,
    FX_BLUR,
    //FX_FXAA
} PostProcessTypes;

void ShaderStore_Init();
void ShaderStore_Release();
Shader & get_shader(ShaderType type) ;
Shader & get_postprocessor(PostProcessTypes type);



Model LoadOldCar();
void PBR_SetAmbientColor (Color ambientColor, Shader & shader) ;
void PBR_SetAmbient(float ambientIntensity, Shader & shader);
void PBR_SetEmissivePower(float emissiveIntensity, Shader & shader);
void PBR_SetEmissiveColor(Color color, Shader & shader);
void PBR_SetTiling(Vector2 tiling, Shader & shader) ;
void PBR_InitializeMaterial(Material & mat) ;
void setCameraPosition(Shader shader, Vector3 v) ;

#endif