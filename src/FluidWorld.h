#pragma once
#include "raylib.h"
#include <vector>

struct FluidParticle
{
    Vector2 position;
    Vector2 velocity;
    float radius;
};

class FluidWorld
{
public:
    int screenWidth = 800;
    int screenHeight = 600;

    void Reset();
    void Update(float dt);
    void Draw();

private:
    std::vector<FluidParticle> particles;

    float gravity = 900.0f;

    void SpawnParticle(Vector2 position);
    void HandleInput();
    void UpdateParticles(float dt);
    void ResolveWallCollisions(FluidParticle& particle);
    void ResolveParticleCollisions();
    float inputDelay = 0.0f;
};