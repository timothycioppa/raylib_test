#ifndef _PARTICLE_SYSTEM_HPP_
#define _PARTICLE_SYSTEM_HPP_

#include "include/raylib.h"

struct ParticleSystem 
{
    unsigned int particleCount;
    Vector3 *positions;
    Vector3 *velocities;
    Matrix  *matrices;
    Vector2 *ageLifetimes;   
};

typedef Vector3 (*ForceExtractor)(Vector3 position);

struct simulation_context
{
    float dt;
    Vector3 cameraPosition;
    ForceExtractor getForce;
};

void ParticleSystemInit() ;
bool AllocateSystem(unsigned int numParticles, ParticleSystem & result) ;
void SimulateSystem(ParticleSystem & particleSystem, simulation_context & context);

#endif