#ifndef _AABB_HPP
#define _AABB_HPP

#include "include/raylib.h"

Vector3 AABB_Center(BoundingBox &box);
Vector3 AABB_Size(BoundingBox &box);
Vector3 AABB_Extents(BoundingBox &box);
void AABB_Update(Vector3 center, Vector3 extents, BoundingBox & bb ) ;

#endif