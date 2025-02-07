#ifndef _CAMERA_HPP_JC
#define _CAMERA_HPP_JC
#include "include/raylib.h"
#include "include/raymath.h"

// wrapper around Camera class, so we can prioritize forward instead of target in our calculataions
struct camera 
{ 
    Camera *cam;
   
    float Horizontal;
    float Vertical;
    Vector3 forward;
    Vector3 right;
    Vector3 up;
};

void rotate_camera(camera & cam) ;
void move_camera(camera & cam, Vector3 offset) ;

#endif