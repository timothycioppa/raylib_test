#include "particle_system.hpp"
#include "include/raymath.h"

#define ARENA_SIZE 4 * 1024 * 1024
unsigned char particleArena[ARENA_SIZE];
unsigned char *arenaHead;
unsigned int allocated;

void ParticleSystemInit() 
{ 
    arenaHead = particleArena;
    allocated = 0;
}

bool AllocateSystem(unsigned int numParticles, ParticleSystem & result) 
{
    int totalSizeRequired = 0;

    totalSizeRequired =  numParticles * sizeof(Vector3); // positions
    totalSizeRequired += numParticles * sizeof(Vector3); // velocities
    totalSizeRequired += numParticles * sizeof(Vector2); // ageLifetimes
    totalSizeRequired += numParticles * sizeof(Matrix);  // transforms

    if (allocated + totalSizeRequired <= ARENA_SIZE)
    {
        result.particleCount = numParticles;
        
        result.positions = (Vector3 *) arenaHead;
        arenaHead += numParticles * sizeof(Vector3);

        result.velocities = (Vector3 *) arenaHead;
        arenaHead += numParticles * sizeof(Vector3);

        result.matrices = (Matrix *) arenaHead;        
        arenaHead += numParticles * sizeof(Matrix);

        result.ageLifetimes = (Vector2 *) arenaHead;        
        arenaHead += numParticles * sizeof(Vector2);

        allocated += totalSizeRequired;

        return true;
    }

    return false;
}

void SimulateSystem(ParticleSystem & particleSystem, simulation_context & context)
{
    for (int i = 0; i < particleSystem.particleCount; i++) 
    {
        Vector3 force = context.getForce(particleSystem.positions[i]);
        particleSystem.velocities[i] += (force * context.dt);
        particleSystem.positions[i] += (particleSystem.velocities[i] * context.dt); 
        particleSystem.matrices[i] = MatrixInvert(MatrixLookAt(particleSystem.positions[i], context.cameraPosition, {0,1,0}));
        particleSystem.ageLifetimes[i].x += context.dt;
    }
}