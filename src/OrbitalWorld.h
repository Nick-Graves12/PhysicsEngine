#pragma once
#include "raylib.h"
#include <vector>

struct OrbitalBody
{
    Vector2 position;
    Vector2 velocity;

    float radius;
    float mass;

    Color color;

    bool isMoon = false;
    int parentIndex = -1;

    std::vector<Vector2> trail;
};

class OrbitalWorld
{
public:
    int screenWidth = 800;
    int screenHeight = 600;
    float spawnRadius = 10.0f;

    void Reset();
    void Update(float dt);
    void Draw();

private:
    OrbitalBody sun;
    std::vector<OrbitalBody> planets;

    bool isDragging = false;
    Vector2 dragStart;
    Vector2 dragCurrent;

    void HandleInput();
    void UpdatePhysics(float dt);

    void DrawTrails();
    void DrawBodies();
    void DrawDragPreview();
    void DrawUI();


    Vector2 CalculateLaunchVelocity() const;

    void SpawnPlanet(Vector2 position, Vector2 velocity);
    void ApplyGravity(OrbitalBody& body,
                  const OrbitalBody& attractor,
                  float dt);

    int selectedPlanetIndex = -1;

    void SelectPlanetAtMouse();
    void DrawSelectedPlanetInfo();
    void SpawnMoon();
    



};