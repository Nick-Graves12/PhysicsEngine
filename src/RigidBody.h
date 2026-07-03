//Stores Data
#pragma once
#include "Vector2D.h"

struct RigidBody
{
    Vector2D velocity;
    Vector2D position;
    Vector2D acceleration;

    float radius = 0.0f;
    float mass = 1.0f;
    float drag = 1.0f;
    float restitution = 1.0f;

    bool isOnGround = false;
    bool isStatic = false;
};