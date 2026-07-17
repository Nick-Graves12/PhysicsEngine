#pragma once
#include <vector>
#include "raylib.h"

struct WindParticle
{
    Vector2 position;
    float radius;
    Color color;
};

class WindWorld
{

public:
    int screenWidth = 1050;
    int screenHeight = 600;

    void Reset();
    void Update(float dt);
    void Draw();

private:
    std::vector<WindParticle> particles;
    void SpawnParticle(Vector2 position);
    void UpdateParticles(float dt);
    void GlobalWindVelocity();

    Rectangle tunnel;
};
