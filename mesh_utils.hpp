#ifndef _MESH_UTILS_HPP
#define _MESH_UTILS_HPP

#include "include/raylib.h"
#include "include/raymath.h"

struct mesh_data_header
{
   unsigned int vertexCount;
   unsigned int tirangleCount;
};

void prepare_mesh_file(Mesh mesh, const char* filename);
void generate_mesh_file(const char* filename);
Mesh load_mesh(const char* filename) ;

#endif