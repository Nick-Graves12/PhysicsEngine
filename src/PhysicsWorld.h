#pragma once

#include <vector>
#include "RigidBody.h"
#include "raylib.h"

struct PhysicsWorld
{
public:
    float gravity = 400.0f;
    int screenWidth = 800;
    int screenHeight = 600;

    void Step(float dt);
    void AddBody(const RigidBody& body);
    void Draw() const;
    RigidBody& GetBody(int index);
    int GetBodyCount() const;
    void Reset();
    void RemoveBody(int index);

private:
    std::vector<RigidBody> bodies;
    Rectangle arena;

    void CheckArenaCollision(RigidBody& body);
};