//Runs Simulation
#include "PhysicsWorld.h"
#include "raylib.h"
#include "Physics.h"
#include "Collision.h"

void PhysicsWorld::Step(float dt)
{
    
    for (RigidBody& body : bodies)
    {
        ResetAcceleration(body);
        ApplyGravity(body, gravity);
        UpdateVelocity(body, dt);
        ApplyDrag(body);
        UpdatePosition(body, dt);
        CheckArenaCollision(body);
    }

    for (int i = 0; i < bodies.size(); i++)
    {
        for (int j = i + 1; j < bodies.size(); j++)
        {
            if (CheckCircleCollision(bodies[i], bodies[j]))
            {
                ResolveCircleCollisionVelocity(bodies[i], bodies[j]);
                ResolveCircleOverlap(bodies[i], bodies[j]);
            }
        }
    }

}

void PhysicsWorld::AddBody(const RigidBody& body)
{
    bodies.push_back(body);
}

void PhysicsWorld::RemoveBody(int index)
{
    if (index < 0 || index >= bodies.size())
    {
        return;
    }

    bodies.erase(bodies.begin() + index);
}

void PhysicsWorld::Draw() const
{   
    DrawRectangleRec(arena, Color{235, 235, 235, 255});
    DrawRectangleLinesEx(arena, 2.0f, LIGHTGRAY);
    DrawRectangleLinesEx(
        {arena.x + 3, arena.y + 3,
        arena.width - 6, arena.height - 6},
        1,
        Fade(GRAY, 0.35f)
    );

    for (int i = 0; i < bodies.size(); i++)
    {
        if (bodies[i].isStatic)
        {
            DrawCircle(bodies[i].position.x, bodies[i].position.y, bodies[i].radius, GREEN);
        }
        else if (i == 0)
        {
            DrawCircle(bodies[i].position.x, bodies[i].position.y, bodies[i].radius, RED);
        }
        else
        {
            DrawCircle(bodies[i].position.x, bodies[i].position.y, bodies[i].radius, BLUE);
        }
    }
}


RigidBody& PhysicsWorld::GetBody(int index)
{
    return bodies[index];
}

int PhysicsWorld::GetBodyCount() const
{
    return bodies.size();
}

void PhysicsWorld::Reset()
{
    bodies.clear();

    arena = {278, 70, 495, 430};

    float startX = arena.x + arena.width / 2.0f;
    float startY = arena.y + arena.height / 2.0f;

    RigidBody player = CreateRigidBody(startX, startY, 30, 1);

    player.velocity.x = 100;
    player.restitution = 0.9f;

    AddBody(player);
}


void PhysicsWorld::CheckArenaCollision(RigidBody& body)
{
    if (body.position.x - body.radius < arena.x)
    {
        body.position.x = arena.x + body.radius;
        body.velocity.x *= -body.restitution;
    }

    if (body.position.x + body.radius > arena.x + arena.width)
    {
        body.position.x = arena.x + arena.width - body.radius;
        body.velocity.x *= -body.restitution;
    }

    if (body.position.y - body.radius < arena.y)
    {
        body.position.y = arena.y + body.radius;
        body.velocity.y *= -body.restitution;
    }

    if (body.position.y + body.radius > arena.y + arena.height)
    {
        body.position.y = arena.y + arena.height - body.radius;
        body.velocity.y *= -body.restitution;
        body.isOnGround = true;
    }
    else
    {
        body.isOnGround = false;
    }
}
