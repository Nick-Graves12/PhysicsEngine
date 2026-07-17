#include "WindWorld.h"
#include "raylib.h"
#include "UI.h"

void WindWorld::Reset()
{
    tunnel.x = 180;
    tunnel.y = 80;
    tunnel.width = 800;
    tunnel.height = 440;

    particles.clear();

    for (int row = 0; row < 84; row++)
    {
        SpawnParticle({
            tunnel.x + 10.0f, tunnel.y + 10.0f + row * 5.0f
        });
    }
}

void WindWorld::Update(float dt)
{

}

void WindWorld::Draw()
{
    ClearBackground(Color{ 225, 235, 245, 255});

    DrawRectangleLinesEx(tunnel, 8.0f, LIGHTGRAY);

    for (const WindParticle& particle : particles)
    {
        DrawCircleV(particle.position, particle.radius, particle.color);
    }

    UIText("R: Reset", 20, 20, 18, GRAY);
    UIText("Backspace: Menu", 20, 570, 18, GRAY);
}

void WindWorld::SpawnParticle(Vector2 position)
{
    WindParticle particle;

    particle.position = position;
    particle.radius = 0.5f;
    particle.color = GRAY;

    particles.push_back(particle);
}