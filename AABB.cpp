#include "AABB.hpp"


Vector3 AABB_Center(BoundingBox &box) 
{
    return 
    {
        box.min.x + 0.5f * (box.max.x - box.min.x),
        box.min.y + 0.5f * (box.max.y - box.min.y),
        box.min.z + 0.5f * (box.max.z - box.min.z)
    };
}

Vector3 AABB_Size(BoundingBox &box)
{
    return 
    {
        box.max.x - box.min.x,
        box.max.y - box.min.y,
        box.max.z - box.min.z
    };
}

Vector3 AABB_Extents(BoundingBox &box)
{
    return 
    {
        0.5f * (box.max.x - box.min.x),
        0.5f * (box.max.y - box.min.y),
        0.5f * (box.max.z - box.min.z)
    };
}
void AABB_Update(Vector3 center, Vector3 extents, BoundingBox & bb ) 
{
    bb.min = {center.x - extents.x, center.y - extents.y, center.z - extents.z} ;
    bb.max = {center.x + extents.x, center.y + extents.y, center.z + extents.z} ;
}