
#include "mesh_utils.hpp"
#include <stdio.h>
#include <string.h>

// load a mesh from the resources/meshes folder. 
Mesh load_mesh(const char* filename) 
{
    int sizeLoaded;
    unsigned char* fileData =  LoadFileData(filename, &sizeLoaded);
    mesh_data_header *pHeader = (mesh_data_header*) fileData;
    unsigned char* pData = fileData + sizeof(mesh_data_header);

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

    UploadMesh(&mesh, false);
    UnloadFileData(fileData);

    return mesh;
}

// void generate_quad_file(const char* filename) 
// { 
//     #include "quad.mesh"
    
//     int headerSize = sizeof(mesh_data_header);

//     int dataSize = 
//         sizeof(TEST_VERTEX_DATA) +  // vec3 vertices
//         sizeof(TEST_TEXCOORD_DATA) +  // vec2 fragcoords
//         sizeof(TEST_NORMAL_DATA) +  // vec3 normals
//         sizeof(TEST_INDEX_DATA) ; // unsigned short
        
//     unsigned char* pData = (unsigned char*) scratch_alloc(headerSize  + dataSize);
    
//     mesh_data_header *pHeader = (mesh_data_header *) pData;
//     pHeader->tirangleCount = TEST_TRIANGLE_COUNT;
//     pHeader->vertexCount = TEST_VERTEX_COUNT; 

//     unsigned char* head = pData + headerSize;

//     memcpy(
//        head,
//         TEST_VERTEX_DATA,  
//         sizeof(TEST_VERTEX_DATA)
//     );

//     head +=  sizeof(TEST_VERTEX_DATA);

//     memcpy(
//         head,
//         TEST_TEXCOORD_DATA,  
//         sizeof(TEST_TEXCOORD_DATA)
//     );

//     head +=     sizeof(TEST_TEXCOORD_DATA);

//     memcpy(
//         head,
//         TEST_NORMAL_DATA,  
//         sizeof(TEST_NORMAL_DATA)
//     );

//     head +=  sizeof(TEST_NORMAL_DATA);
    
//     memcpy(
//         head,
//         TEST_INDEX_DATA,  
//         sizeof(TEST_INDEX_DATA)
//     );

//     SaveFileData(filename, (void*) pData, headerSize + dataSize);
// }


// void generate_cube_file(const char* filename) 
// { 
//     #include "cube.mesh"
    
//     int headerSize = sizeof(mesh_data_header);

//     int dataSize = 
//         sizeof(CUBE_VERTEX_DATA) +  // vec3 vertices
//         sizeof(CUBE_TEXCOORD_DATA) +  // vec2 fragcoords
//         sizeof(CUBE_NORMAL_DATA) +  // vec3 normals
//         sizeof(CUBE_INDEX_DATA) ; // unsigned short
        
//     unsigned char* pData = (unsigned char*) scratch_alloc(headerSize  + dataSize);
    
//     mesh_data_header *pHeader = (mesh_data_header *) pData;
//     pHeader->tirangleCount = CUBE_TRIANGLE_COUNT;
//     pHeader->vertexCount = CUBE_VERTEX_COUNT; 

//     unsigned char* head = pData + headerSize;

//     memcpy(
//        head,
//        CUBE_VERTEX_DATA,  
//         sizeof(CUBE_VERTEX_DATA)
//     );

//     head +=  sizeof(CUBE_VERTEX_DATA);

//     memcpy(
//         head,
//         CUBE_TEXCOORD_DATA,  
//         sizeof(CUBE_TEXCOORD_DATA)
//     );

//     head +=     sizeof(CUBE_TEXCOORD_DATA);

//     memcpy(
//         head,
//         CUBE_NORMAL_DATA,  
//         sizeof(CUBE_NORMAL_DATA)
//     );

//     head +=  sizeof(CUBE_NORMAL_DATA);
    
//     memcpy(
//         head,
//         CUBE_INDEX_DATA,  
//         sizeof(CUBE_INDEX_DATA)
//     );

//     SaveFileData(filename, (void*) pData, headerSize + dataSize);
// }