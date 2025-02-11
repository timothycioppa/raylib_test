#include "shader_store.hpp"
#include "scratch.hpp"
#include <stdio.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

Shader shaders[ShaderType::SHADER_COUNT];
Shader postProcessors[12];

void load_shader(const char* name, Shader & s) 
{
    char* vbuff = (char*) scratch_alloc(128);
    sprintf(vbuff, "resources/shaders/glsl%i/%s.vs", GLSL_VERSION, name);
    
    char* fbuff = (char*) scratch_alloc(128);
    sprintf(fbuff, "resources/shaders/glsl%i/%s.fs", GLSL_VERSION, name);
    
    s = LoadShader(vbuff, fbuff);
} 

void load_shader(const char* namev, const char* namef, Shader & s) 
{
    char* vbuff = (char*) scratch_alloc(128);
    sprintf(vbuff, "resources/shaders/glsl%i/%s.vs", GLSL_VERSION, namev);
    
    char* fbuff = (char*) scratch_alloc(128);
    sprintf(fbuff, "resources/shaders/glsl%i/%s.fs", GLSL_VERSION, namef);
    
    s = LoadShader(vbuff, fbuff);
}

void setCameraPosition(Shader shader, Vector3 v) 
{
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &v, SHADER_UNIFORM_VEC3);
}


void PBR_SetAmbientColor (Color ambientColor, Shader & shader) 
{ 
    Vector3 ambientColorNormalized = (Vector3){ ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f };
    SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambientColorNormalized, SHADER_UNIFORM_VEC3);
}

void PBR_SetAmbient(float ambientIntensity, Shader & shader) {
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientIntensity, SHADER_UNIFORM_FLOAT);
}

void PBR_SetEmissivePower(float emissiveIntensity, Shader & shader)
{
    SetShaderValue(shader, GetShaderLocation(shader, "emissivePower"), &emissiveIntensity, SHADER_UNIFORM_FLOAT); 
}

void PBR_SetEmissiveColor(Color color, Shader & shader)
{
    Vector3 colorNormalize = (Vector3){ color.r/255.0f, color.g/255.0f, color.b/255.0f };
    SetShaderValue(shader, GetShaderLocation(shader, "emissiveColor"), &colorNormalize, SHADER_UNIFORM_VEC3); 
}

void PBR_SetTiling(Vector2 tiling, Shader & shader) 
{ 
    SetShaderValue(shader, GetShaderLocation(shader, "tiling"), &tiling, SHADER_UNIFORM_VEC2);
}

void PBR_InitializeMaterial(Material & mat) 
{ 
    mat.shader = get_shader(ShaderType::SHADER_PBR);
    mat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    mat.maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    mat.maps[MATERIAL_MAP_ROUGHNESS].value = 0.0f;
    mat.maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    mat.maps[MATERIAL_MAP_EMISSION].color = (Color){ 255, 162, 0, 255 };
   
    mat.maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("resources/textures/old_car_d.png");
    mat.maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("resources/textures/old_car_mra.png");
    mat.maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("resources/textures/old_car_n.png");
    mat.maps[MATERIAL_MAP_EMISSION].texture = LoadTexture("resources/textures/old_car_e.png");

    int usage = 1;
    SetShaderValue( mat.shader, GetShaderLocation( mat.shader, "useTexAlbedo"), &usage, SHADER_UNIFORM_INT);
    SetShaderValue( mat.shader, GetShaderLocation( mat.shader, "useTexNormal"), &usage, SHADER_UNIFORM_INT);
    SetShaderValue( mat.shader, GetShaderLocation( mat.shader, "useTexMRA"), &usage, SHADER_UNIFORM_INT);
    SetShaderValue( mat.shader, GetShaderLocation( mat.shader, "useTexEmissive"), &usage, SHADER_UNIFORM_INT);

  

}

Model LoadOldCar()
{
    Model model = LoadModel("resources/models/old_car_new.glb");
    PBR_InitializeMaterial(model.materials[0]);
    return model;
}

void setCameraPosition(Camera & camera, Shader &shader)
{
    float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
}

void ShaderStore_Init() 
{ 
    Shader & lit = shaders[int(ShaderType::SHADER_LIT)];
    load_shader("lighting", lit);
    lit.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lit, "viewPos");

    Shader & inst = shaders[int(ShaderType::SHADER_LIT_INSTANCED)];
    load_shader("lighting_instancing", "lighting", inst);
    inst.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(inst, "viewPos");
    inst.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(inst, "mvp");

    Shader & pbr = shaders[int(ShaderType::SHADER_PBR)];
    load_shader("pbr", "pbr", pbr);
    pbr.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(pbr, "viewPos");
    pbr.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(pbr, "albedoMap");
    pbr.locs[SHADER_LOC_MAP_METALNESS] = GetShaderLocation(pbr, "mraMap");
    pbr.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(pbr, "normalMap");
    pbr.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(pbr, "emissiveMap");
    pbr.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(pbr, "albedoColor");


    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    postProcessors[FX_GRAYSCALE] = LoadShader(0, TextFormat("resources/shaders/glsl%i/grayscale.fs", GLSL_VERSION));
    postProcessors[FX_POSTERIZATION] = LoadShader(0, TextFormat("resources/shaders/glsl%i/posterization.fs", GLSL_VERSION));
    postProcessors[FX_DREAM_VISION] = LoadShader(0, TextFormat("resources/shaders/glsl%i/dream_vision.fs", GLSL_VERSION));
    postProcessors[FX_PIXELIZER] = LoadShader(0, TextFormat("resources/shaders/glsl%i/pixelizer.fs", GLSL_VERSION));
    postProcessors[FX_CROSS_HATCHING] = LoadShader(0, TextFormat("resources/shaders/glsl%i/cross_hatching.fs", GLSL_VERSION));
    postProcessors[FX_CROSS_STITCHING] = LoadShader(0, TextFormat("resources/shaders/glsl%i/cross_stitching.fs", GLSL_VERSION));
    postProcessors[FX_PREDATOR_VIEW] = LoadShader(0, TextFormat("resources/shaders/glsl%i/predator.fs", GLSL_VERSION));
    postProcessors[FX_SCANLINES] = LoadShader(0, TextFormat("resources/shaders/glsl%i/scanlines.fs", GLSL_VERSION));
    postProcessors[FX_FISHEYE] = LoadShader(0, TextFormat("resources/shaders/glsl%i/fisheye.fs", GLSL_VERSION));
    postProcessors[FX_SOBEL] = LoadShader(0, TextFormat("resources/shaders/glsl%i/sobel.fs", GLSL_VERSION));
    postProcessors[FX_BLOOM] = LoadShader(0, TextFormat("resources/shaders/glsl%i/bloom.fs", GLSL_VERSION));
    postProcessors[FX_BLUR] = LoadShader(0, TextFormat("resources/shaders/glsl%i/blur.fs", GLSL_VERSION));

}

void ShaderStore_Release() 
{ 
    UnloadShader(shaders[int(ShaderType::SHADER_LIT)]);
    UnloadShader(shaders[int(ShaderType::SHADER_LIT_INSTANCED)]);
    UnloadShader(shaders[int(ShaderType::SHADER_PBR)]);


    UnloadShader(postProcessors[FX_GRAYSCALE]);
    UnloadShader(postProcessors[FX_POSTERIZATION]);
    UnloadShader(postProcessors[FX_DREAM_VISION]);
    UnloadShader(postProcessors[FX_PIXELIZER]);
    UnloadShader(postProcessors[FX_CROSS_HATCHING]);
    UnloadShader(postProcessors[FX_CROSS_STITCHING]);
    UnloadShader(postProcessors[FX_PREDATOR_VIEW]);
    UnloadShader(postProcessors[FX_SCANLINES]);
    UnloadShader(postProcessors[FX_FISHEYE]);
    UnloadShader(postProcessors[FX_SOBEL]);
    UnloadShader(postProcessors[FX_BLOOM]);
    UnloadShader(postProcessors[FX_BLUR]);
}

Shader & get_shader(ShaderType type) 
{  
    return shaders[int(type)];
}

Shader & get_postprocessor(PostProcessTypes type) 
{  
    return postProcessors[int(type)];
}