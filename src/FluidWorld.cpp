#include "FluidWorld.h"
#include <cmath>

void FluidWorld::Reset()
{
    particles.clear();
    for (int row = 0; row < 16; row++)
    {
        for (int col = 0; col < 70; col++)
        {
            SpawnParticle({
                90.0f + col * 6.0f + GetRandomValue(-1, 1),
                360.0f + row * 6.0f + GetRandomValue(-1, 1)
            });
        }
    }
}

void FluidWorld::Update(float dt)
{
    if (inputDelay > 0.0f)
    {
        inputDelay -= dt;
    }

    HandleInput();
    UpdateParticles(dt);

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

    DrawText("Fluid Sim", 20, 20, 30, WHITE);
    DrawText("Hold left click: pour particles", 20, 60, 20, GRAY);
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

void FluidWorld::ResolveWallCollisions(FluidParticle& particle)
{
    float restitution = 0.3f;

    if (particle.position.x - particle.radius < 0)
    {
        particle.position.x = particle.radius;
        particle.velocity.x *= -restitution;
    }

    if (particle.position.x + particle.radius > screenWidth)
    {
        particle.position.x = screenWidth - particle.radius;
        particle.velocity.x *= -restitution;
    }

    if (particle.position.y - particle.radius < 0)
    {
        particle.position.y = particle.radius;
        particle.velocity.y *= -restitution;
    }

    if (particle.position.y + particle.radius > screenHeight)
    {
        particle.position.y = screenHeight - particle.radius;
        particle.velocity.y *= -restitution;
    }


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