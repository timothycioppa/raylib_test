#ifndef _RAYLIB_EXTENSIONS_HPP
#define _RAYLIB_EXTENSIONS_HPP


#include "include/raylib.h"
#include "include/rlgl.h"

// like drawgrid, but you can specify the origin
void DrawGrid(int slices, float spacing, Vector3 center);
unsigned char *LoadFileData_scratch(const char *fileName, int *dataSize);

#endif