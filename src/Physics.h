#pragma once

#include "RigidBody.h"

void ApplyDrag(RigidBody& body);
void ApplyGravity(RigidBody& body, float gravity);
void UpdatePosition(RigidBody& body, float dt);
void UpdateVelocity(RigidBody& body, float dt);
void ResetAcceleration(RigidBody& body);
void CheckRigidBodyScreenCollision(RigidBody& body,
                                   int screenWidth,
                                   int screenHeight);

RigidBody CreateRigidBody(float x,
                          float y,
                          float radius,
                          float mass);
void ApplyForce(RigidBody& body, Vector2D force);
void ApplyImpulse(RigidBody& body, Vector2D impulse);