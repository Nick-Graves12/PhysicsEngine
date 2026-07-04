#pragma once
#include "raylib.h"
#include <vector>

struct FluidParticle
{
    Vector2 position;
    Vector2 velocity;
    float radius;
};

struct FluidBox
{
    Vector2 position;
    Vector2 velocity;

    float width;
    float height;

    float rotation;
    float angularVelocity;

    float mass;
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

    std::vector<FluidBox> boxes;
    Rectangle tank;

    void SpawnBox(Vector2 position);
    void UpdateBoxes(float dt);
    void ResolveBoxWallCollisions(FluidBox& box);
    void ApplyBuoyancy(FluidBox& box);
};