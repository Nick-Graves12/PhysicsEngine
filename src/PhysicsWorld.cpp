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
        CheckRigidBodyScreenCollision(body, screenWidth, screenHeight);
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

void PhysicsWorld::Draw() const
{
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

    RigidBody player = CreateRigidBody(400, 300, 30, 1);
    player.velocity.x = 100;
    player.restitution = 0.9f;

    AddBody(player);
}