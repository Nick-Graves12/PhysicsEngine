#pragma once
#include "raylib.h"
#include <vector>

struct FluidParticle
{
    Vector2 position;
    Vector2 velocity;
    float radius;
    float density = 0.0f;
    float pressure = 0.0f;
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

    ~FluidWorld();

    void Reset();
    void Update(float dt);
    void Draw();

private:
    std::vector<FluidParticle> particles;

    float gravity = 900.0f;

    float gridCellSize = 14.0f;
    int gridColumns = 0;
    int gridRows = 0;

    RenderTexture2D fluidRenderTexture;
    Shader metaballShader;

    bool fluidRenderingLoaded = false;

    void InitializeFluidRendering();
    void DrawMetaballFluid();

    std::vector<std::vector<int>> spatialGrid;

    void InitializeSpatialGrid();
    void BuildSpatialGrid();
    int GetGridIndex(Vector2 position) const;

    void SpawnParticle(Vector2 position);
    void HandleInput();
    void UpdateParticles(float dt);

    void ResolveWallCollisions(FluidParticle& particle);
    void ResolveParticleCollisions();
    void ResolveParticleBoxCollisions();

    void ApplyViscosity();
    void ComputeDensity();
    void ApplyPressure(float dt);

    float inputDelay = 0.0f;
    int viscosityFrameCounter = 0;
    int densityFrameCounter = 0;

    std::vector<FluidBox> boxes;
    Rectangle tank;

    void SpawnBox(Vector2 position);
    void UpdateBoxes(float dt);
    void ResolveBoxWallCollisions(FluidBox& box);
    void ApplyBuoyancy(FluidBox& box);

   

};