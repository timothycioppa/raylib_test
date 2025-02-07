#include "camera.hpp"

void rotate_camera(camera & cam) 
{   
    float yawRad = DEG2RAD * (cam.Horizontal);
    float pitchRad = DEG2RAD * (cam.Vertical);
    Vector3 &front = cam.forward;

    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);

    cam.forward = Vector3Normalize(front);
    cam.right = Vector3Normalize(Vector3CrossProduct({0,1,0}, cam.forward));  
    cam.up    = Vector3Normalize(Vector3CrossProduct( cam.forward, cam.right));
    
    Camera * _cam = cam.cam;
    _cam->target = _cam->position + cam.forward;
    _cam->up = cam.up;
}

void move_camera(camera & cam, Vector3 offset) 
{
    Camera * _cam = cam.cam;
    _cam->target = _cam->target + offset;
    _cam->position = _cam->position + offset;
}