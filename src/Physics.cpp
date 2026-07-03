//Applied Forces and updates motion
#include "Physics.h"

void ApplyDrag(RigidBody& body)
{
    body.velocity.x *= body.drag;
}

void ApplyGravity(RigidBody& body, float gravity)
{
    ApplyForce(body, Vector2D{0, body.mass * gravity});
}

void UpdatePosition(RigidBody& body, float dt)
{
     if (body.isStatic)
    {
        return;
    }
    body.position.x += body.velocity.x * dt;
    body.position.y += body.velocity.y * dt;
}

void UpdateVelocity(RigidBody& body, float dt)
{
     if (body.isStatic)
    {
        return;
    }
    body.velocity.x += body.acceleration.x * dt;
    body.velocity.y += body.acceleration.y * dt;
}

void ResetAcceleration(RigidBody& body)
{
    body.acceleration.x = 0;
    body.acceleration.y = 0;
}

void CheckRigidBodyScreenCollision(RigidBody& body, int screenWidth, int screenHeight)
{
    if (body.position.y + body.radius >= screenHeight)
    {
        body.position.y = screenHeight - body.radius;
        body.velocity.y = -body.velocity.y * body.restitution;
        body.isOnGround = true;
    }
    else
    {
        body.isOnGround = false;
    }

    if (body.position.x + body.radius >= screenWidth)
    {
        body.position.x = screenWidth - body.radius;
        body.velocity.x = -body.velocity.x;
    }

    if (body.position.x - body.radius <= 0)
    {
        body.position.x = body.radius;
        body.velocity.x = -body.velocity.x;
    }
}

RigidBody CreateRigidBody(float x, float y, float radius, float mass)
{
    RigidBody body;
    body.position.x = x;
    body.position.y = y;
    body.radius = radius;
    body.mass = mass;

    body.velocity.x = 0;
    body.velocity.y = 0;
    body.acceleration.x = 0;
    body.acceleration.y = 0;

    body.drag = 0.999f;
    body.restitution = 0.8f;
    body.isOnGround = false;

    return body;
}
void ApplyForce(RigidBody& body, Vector2D force)
{
    if (body.isStatic)
    {
        return;
    }
    body.acceleration.x += force.x / body.mass;
    body.acceleration.y += force.y / body.mass;
}
void ApplyImpulse(RigidBody& body, Vector2D impulse)
{
     if (body.isStatic)
    {
        return;
    }
    body.velocity.x += impulse.x / body.mass;
    body.velocity.y += impulse.y / body.mass;
}