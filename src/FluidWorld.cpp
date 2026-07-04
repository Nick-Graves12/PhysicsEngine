#include "FluidWorld.h"
#include <cmath>

void FluidWorld::Reset()
{
    tank.x = 180;
    tank.y = 120;
    tank.width = 440;
    tank.height = 480;
    particles.clear();
    for (int row = 0; row < 24; row++)
    {
        for (int col = 0; col < 55; col++)
        {
            float waterDepth = 170.0f;
            SpawnParticle({
                tank.x + 40.0f + col * 6.0f + GetRandomValue(-1, 1),
                tank.y + tank.height - waterDepth + row * 6.0f + GetRandomValue(-3, 3)
            });
        }
    }

    boxes.clear();

    SpawnBox({
        tank.x + tank.width / 2.0f,
        tank.y + 60.0f
    });
}

void FluidWorld::Update(float dt)
{

    HandleInput();
    UpdateParticles(dt);
    UpdateBoxes(dt);

}

void FluidWorld::Draw()
{
    ClearBackground(BLACK);

    for (const FluidParticle& particle : particles)
    {
        DrawCircleV(
            particle.position,
            particle.radius + 1.5f,
            Fade(SKYBLUE, 0.45f)
        );

        DrawCircleV(
            particle.position,
            particle.radius,
            Fade(BLUE, 0.35f)
        );
    }

    for (const FluidBox& box : boxes)
    {
        Rectangle rect = {
            box.position.x,
            box.position.y,
            box.width,
            box.height
        };

        DrawRectanglePro(
            rect,
            { box.width / 2.0f, box.height / 2.0f },
            box.rotation,
            ORANGE
        );
    }
    DrawRectangleLinesEx(tank, 8.0f, LIGHTGRAY);

    DrawText("Fluid Sim", 20, 20, 30, WHITE);
    DrawText("Floating box buoyancy test", 20, 60, 20, GRAY);
    DrawText("R: Reset", 20, 90, 20, GRAY);
    DrawText("BACKSPACE: Menu", 20, 560, 20, GRAY);
}

void FluidWorld::HandleInput()
{
   /*  if (inputDelay > 0.0f)
    {
        return;
    } */
}

void FluidWorld::SpawnParticle(Vector2 position)
{
    FluidParticle particle;

    particle.position = position;
    particle.velocity = { 0.0f, 0.0f };
    particle.radius = 2.5f;

    particles.push_back(particle);
}

void FluidWorld::UpdateParticles(float dt)
{
   for (FluidParticle& particle : particles)
    {
        particle.velocity.y += gravity * dt;

        particle.position.x += particle.velocity.x * dt;
        particle.position.y += particle.velocity.y * dt;

       
        particle.velocity.x *= 0.999f;
        particle.velocity.y *= 0.999f;

        ResolveWallCollisions(particle);
    }

    ResolveParticleCollisions();
}

void FluidWorld::ResolveParticleCollisions()
{
    for (int i = 0; i < particles.size(); i++)
    {
        for (int j = i + 1; j < particles.size(); j++)
        {
            FluidParticle& a = particles[i];
            FluidParticle& b = particles[j];

            float dx = b.position.x - a.position.x;
            float dy = b.position.y - a.position.y;

            float distanceSquared = dx * dx + dy * dy;
            float minDistance = a.radius + b.radius;

            if (distanceSquared < minDistance * minDistance)
            {
                float distance = sqrt(distanceSquared);

                if (distance == 0.0f)
                {
                    continue;
                }

                float normalX = dx / distance;
                float normalY = dy / distance;

                float overlap = minDistance - distance;

                a.position.x -= normalX * overlap * 0.5f;
                a.position.y -= normalY * overlap * 0.5f;

                b.position.x += normalX * overlap * 0.5f;
                b.position.y += normalY * overlap * 0.5f;

                float tempX = a.velocity.x;
                float tempY = a.velocity.y;

                a.velocity.x = (a.velocity.x + b.velocity.x) * 0.5f;
                a.velocity.y = (a.velocity.y + b.velocity.y) * 0.5f;

                b.velocity.x = (tempX + b.velocity.x) * 0.5f;
                b.velocity.y = (tempY + b.velocity.y) * 0.5f;
                            
            }
        }
    }
}

void FluidWorld::SpawnBox(Vector2 position)
{
    FluidBox box;

    box.position = position;
    box.velocity = { 0.0f, 0.0f };

    box.width = 70.0f;
    box.height = 70.0f;

    box.rotation = -10.0f;
    box.angularVelocity = 0.0f;

    box.mass = 5.0f;

    boxes.push_back(box);
}

void FluidWorld::UpdateBoxes(float dt)
{
    for (FluidBox& box : boxes)
    {
        box.velocity.y += gravity * dt;

        box.position.x += box.velocity.x * dt;
        box.position.y += box.velocity.y * dt;

        box.rotation += box.angularVelocity * dt;
        ApplyBuoyancy(box);
        box.velocity.x *= 0.995f;
        box.velocity.y *= 0.995f;
        box.angularVelocity *= 0.98f;

        if (box.angularVelocity > 20.0f)
        {
            box.angularVelocity = 20.0f;
        }

        if (box.angularVelocity < -20.0f)
        {
            box.angularVelocity = -20.0f;
        }

        ResolveBoxWallCollisions(box);
    }
}

void FluidWorld::ResolveWallCollisions(FluidParticle& particle)
{
    float restitution = 0.3f;
    float visualRadius = particle.radius + 1.5f;

    if (particle.position.x - visualRadius < tank.x)
    {
        particle.position.x = tank.x + visualRadius;
        particle.velocity.x *= -restitution;
    }

    if (particle.position.x + visualRadius > tank.x + tank.width)
    {
        particle.position.x = tank.x + tank.width - visualRadius;
        particle.velocity.x *= -restitution;
    }

    if (particle.position.y - visualRadius < tank.y)
    {
        particle.position.y = tank.y + visualRadius;
        particle.velocity.y *= -restitution;
    }

    if (particle.position.y + visualRadius > tank.y + tank.height)
    {
        particle.position.y = tank.y + tank.height - visualRadius;
        particle.velocity.y *= -restitution;
    }
}

void FluidWorld::ApplyBuoyancy(FluidBox& box)
{
    float halfWidth = box.width / 2.0f;
    float halfHeight = box.height / 2.0f;

    for (FluidParticle& particle : particles)
    {
        if (particle.position.x > box.position.x - halfWidth &&
            particle.position.x < box.position.x + halfWidth &&
            particle.position.y > box.position.y &&
            particle.position.y < box.position.y + halfHeight)
        {
            // Push the box upward
            float depth = particle.position.y - box.position.y;
            float submersion = depth / halfHeight;

            if (submersion > 1.0f)
            {
                submersion = 1.0f;
            }
            box.velocity.y -= 0.08f * submersion;
            // Push the particle downward
            particle.velocity.y += 0.04f * submersion;

            float sideOffset = particle.position.x - box.position.x;

            box.angularVelocity += sideOffset * 0.000005f * submersion;
        }
    }
}

void FluidWorld::ResolveBoxWallCollisions(FluidBox& box)
{
    float restitution = 0.3f;

    float halfWidth = box.width / 2.0f;
    float halfHeight = box.height / 2.0f;

    if (box.position.x - halfWidth < tank.x)
    {
        box.position.x = tank.x + halfWidth;
        box.velocity.x *= -restitution;
    }

    if (box.position.x + halfWidth > tank.x + tank.width)
    {
        box.position.x = tank.x + tank.width - halfWidth;
        box.velocity.x *= -restitution;
    }

    if (box.position.y - halfHeight < tank.y)
    {
        box.position.y = tank.y + halfHeight;
        box.velocity.y *= -restitution;
    }

    if (box.position.y + halfHeight > tank.y + tank.height)
    {
        box.position.y = tank.y + tank.height - halfHeight;
        box.velocity.y *= -restitution;
        box.angularVelocity = 20.0f;
    }
}