#ifndef _MESH_UTILS_HPP
#define _MESH_UTILS_HPP

#include "include/raylib.h"
#include "include/raymath.h"

struct mesh_data_header
{
   unsigned int vertexCount;
   unsigned int tirangleCount;
};

Mesh load_mesh(const char* filename) ;

#endif