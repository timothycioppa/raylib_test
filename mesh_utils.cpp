
#include "mesh_utils.hpp"
#include <stdio.h>
#include <string.h>
#include "scratch.hpp"
#include "../raylib_extensions.hpp"

// load a mesh from the resources/meshes folder. 
Mesh load_mesh(const char* meshName) 
{
    int sizeLoaded;

    char* path = (char*) scratch_alloc(256);
    sprintf(path, "resources/meshes/%s_compressed", meshName);
    unsigned char* fileData =  LoadFileData_scratch(path, &sizeLoaded);
    
    int decompressedSize;
    unsigned char* meshData = DecompressData(fileData, sizeLoaded, &decompressedSize);

    mesh_data_header *pHeader = (mesh_data_header*) meshData;
    unsigned char* pData = meshData + sizeof(mesh_data_header);

    Mesh mesh = {0};
    mesh.vertexCount = pHeader->vertexCount;
    mesh.triangleCount =  pHeader->tirangleCount;

    // initialize to start of data section
    Vector3 *vertices = (Vector3*) pData;
    pData += (3 * mesh.vertexCount * sizeof(float));
    
    Vector2 *texcoords = (Vector2*) pData;
    pData += (2 * mesh.vertexCount * sizeof(float));
    
    Vector3 *normals = (Vector3*) pData;
    pData += (3 * mesh.vertexCount * sizeof(float));
    
    unsigned short *triangles = (unsigned short* ) pData;


    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    memcpy(mesh.vertices, vertices, 3 * mesh.vertexCount * sizeof(float));
    
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    memcpy(mesh.texcoords, texcoords, 2 * mesh.vertexCount * sizeof(float));
    
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    memcpy(mesh.normals, normals, 3 * mesh.vertexCount * sizeof(float));
    
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));
    memcpy(mesh.indices, triangles, mesh.triangleCount * 3 * sizeof(unsigned short));

    MemFree(meshData);
    UploadMesh(&mesh, false);

    return mesh;
}


void prepare_mesh_file(Mesh mesh, const char* meshName) 
{ 
    char* path = (char*) scratch_alloc(256);
    sprintf(path, "%s.mesh", meshName);
    ExportMeshAsCode(mesh, path);
}

void generate_mesh_file(const char* meshName) 
{ 
    #define VERTEX_COUNT(name) name##_VERTEX_COUNT
    #define TRIANGLE_COUNT(name) name##_TRIANGLE_COUNT
    #define VERTEX_DATA(name) name##_VERTEX_DATA
    #define NORMAL_DATA(name) name##_TEXCOORD_DATA
    #define TEXCOORD_DATA(name) name##_NORMAL_DATA
    #define INDEX_DATA(name) name##_INDEX_DATA

    // modify this data to fit the mesh save file before loading
    #define NUMVERTICES VERTEX_COUNT(CUBE)
    #define NUMTRIANGLES TRIANGLE_COUNT(CUBE)
    #define VERTICES VERTEX_DATA(CUBE)
    #define TEXCOORDS TEXCOORD_DATA(CUBE)
    #define NORMALS NORMAL_DATA(CUBE)
    #define TRIANGLES INDEX_DATA(CUBE)
    #define FILE_NAME "cube.mesh"

    // nothign here should need modifying, only the block above 
    #include FILE_NAME
    int headerSize = sizeof(mesh_data_header);

    int dataSize = 
        sizeof(VERTICES) +      // vec3 vertices
        sizeof(TEXCOORDS) +     // vec2 fragcoords
        sizeof(NORMALS) +       // vec3 normals
        sizeof(TRIANGLES) ;     // unsigned short
        
    unsigned char* pData = (unsigned char*) scratch_alloc(headerSize  + dataSize);

    mesh_data_header *pHeader = (mesh_data_header *) pData;
    pHeader->tirangleCount = NUMTRIANGLES;
    pHeader->vertexCount = NUMVERTICES; 

    unsigned char* head = pData + headerSize;

    memcpy(head, VERTICES, sizeof(VERTICES));
    head +=  sizeof(VERTICES);

    memcpy( head, TEXCOORDS, sizeof(TEXCOORDS));
    head += sizeof(TEXCOORDS);

    memcpy(  head, NORMALS, sizeof(NORMALS));
    head += sizeof(NORMALS);

    memcpy(head, TRIANGLES, sizeof(TRIANGLES));

    char* path = (char*) scratch_alloc(256);
    sprintf(path, "resources/meshes/%s_compressed", meshName);

    int compressedSize;
    unsigned char* compressedData = CompressData(pData, headerSize + dataSize, &compressedSize);

    SaveFileData(path, (void*) compressedData, compressedSize);
    MemFree(compressedData);

}
