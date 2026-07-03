#pragma once

#include "RigidBody.h"

float DistanceSquared(const RigidBody& a, const RigidBody& b);

bool CheckCircleCollision(const RigidBody& a,
                          const RigidBody& b);

void ResolveCircleOverlap(RigidBody& a,
                          RigidBody& b);

void ResolveCircleCollisionVelocity(RigidBody& a,
                                    RigidBody& b);