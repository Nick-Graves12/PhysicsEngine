//Handles Collisions
#include "Collision.h"
#include "Physics.h"
#include <cmath>

float DistanceSquared(const RigidBody& a, const RigidBody& b)
{
    float dx = b.position.x - a.position.x;
    float dy = b.position.y - a.position.y;
    
    return dx * dx + dy * dy;
}


bool CheckCircleCollision(const RigidBody& a, const RigidBody& b)
{
    float radiusSum = a.radius + b.radius;
    return DistanceSquared(a, b) <= radiusSum * radiusSum;
}

void ResolveCircleOverlap(RigidBody& a, RigidBody& b)
{
    float dx = b.position.x - a.position.x;
    float dy = b.position.y - a.position.y;

    float distance = sqrt(dx * dx + dy * dy);

    if (distance == 0)
    {
        return;
    }

    float radiusSum = a.radius + b.radius;
    float penetration = radiusSum - distance;
    if (penetration > 0)
    {
        float normalX = dx / distance;
        float normalY = dy / distance;

        float inverseMassA = 1.0f / a.mass;
        float inverseMassB = 1.0f / b.mass;
        float inverseMassSum = inverseMassA + inverseMassB;

        if (inverseMassSum == 0.0f)
        {
            return;
        }

        float moveA = penetration * (inverseMassA / inverseMassSum);
        float moveB = penetration * (inverseMassB / inverseMassSum);

        if (a.isStatic && !b.isStatic)
        {
            b.position.x += normalX * penetration;
            b.position.y += normalY * penetration;
        }
        else if (!a.isStatic && b.isStatic)
        {
            a.position.x -= normalX * penetration;
            a.position.y -= normalY * penetration;
        }
        else if (!a.isStatic && !b.isStatic)
        {
            a.position.x -= normalX * penetration * 0.5f;
            a.position.y -= normalY * penetration * 0.5f;

            b.position.x += normalX * penetration * 0.5f;
            b.position.y += normalY * penetration * 0.5f;
        }
    }
}

void ResolveCircleCollisionVelocity(RigidBody& a, RigidBody& b)
{
    float dx = b.position.x - a.position.x;
    float dy = b.position.y - a.position.y;

    float distance = sqrt(dx * dx + dy * dy);

    if (distance == 0)
    {
        return;
    }

    // Collision normal (unit vector)
    float normalX = dx / distance;
    float normalY = dy / distance;

    // Relative velocity of B with respect to A
    float relativeVelocityX = b.velocity.x - a.velocity.x;
    float relativeVelocityY = b.velocity.y - a.velocity.y;

    // Velocity along the collision normal (dot product)
    float velocityAlongNormal =
        relativeVelocityX * normalX +
        relativeVelocityY * normalY;

    // Bodies are already moving apart
    if (velocityAlongNormal > 0)
    {
        return;
    }

    // Average restitution (bounciness)
    float restitution = (a.restitution + b.restitution) * 0.5f;

    // Calculate scalar impulse
    float impulse = -(1.0f + restitution) * velocityAlongNormal;
    impulse /= (1.0f / a.mass) + (1.0f / b.mass);

    // Convert scalar impulse into a vector
    float impulseX = impulse * normalX;
    float impulseY = impulse * normalY;

    // Apply impulse
    ApplyImpulse(a, Vector2D{-impulseX, -impulseY});
    ApplyImpulse(b, Vector2D{ impulseX,  impulseY});
}