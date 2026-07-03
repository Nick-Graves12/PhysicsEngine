#include "OrbitalWorld.h"
#include <cmath>

void OrbitalWorld::Reset()
{   
    planets.clear();

    sun.position = {
        screenWidth / 2.0f,
        screenHeight / 2.0f
    };

    sun.velocity = { 0.0f, 0.0f };
    sun.radius = 35.0f;
    sun.mass = 10000.0f;
    sun.color = YELLOW;

   
};

void OrbitalWorld::HandleInput()
{
    if (IsKeyPressed(KEY_Q))
    {
        Vector2 position = GetMousePosition();

        float G = 500.0f;

        float dx = position.x - sun.position.x;
        float dy = position.y - sun.position.y;

        float distance = sqrt(dx * dx + dy * dy);

        if (distance > sun.radius + 20.0f)
        {
            float orbitSpeed = sqrt((G * sun.mass) / distance);

            Vector2 velocity;
            velocity.x = -dy / distance * orbitSpeed;
            velocity.y =  dx / distance * orbitSpeed;

            SpawnPlanet(position, velocity);
        }
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        dragStart = GetMousePosition();
        dragCurrent = dragStart;
        isDragging = true;
    }
    if (isDragging)
    {
        dragCurrent = GetMousePosition();
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging)
    {
        Vector2 launchVelocity;

        float launchScale = 0.5f;

        launchVelocity.x = (dragStart.x - dragCurrent.x) * launchScale;
        launchVelocity.y = (dragStart.y - dragCurrent.y) * launchScale;

        SpawnPlanet(dragStart, launchVelocity);

        isDragging = false;
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        SelectPlanetAtMouse();
    }

    if (IsKeyPressed(KEY_LEFT_BRACKET))
    {
        if (spawnRadius > 5.0f)
        {
            spawnRadius -= 1.0f;
        }
    }

    if (IsKeyPressed(KEY_RIGHT_BRACKET))
    {
        if (spawnRadius < 50.0f)
        {
            spawnRadius += 1.0f;
        }
    }

    /* if (IsKeyPressed(KEY_M))
    {
        SpawnMoon();
    } */
}

void OrbitalWorld::UpdatePhysics(float dt)
{
    for (OrbitalBody& planet : planets)
{
    // Sun always pulls
    ApplyGravity(planet, sun, dt);

    // If this is a moon, its parent also pulls it
    if (planet.isMoon && planet.parentIndex != -1)
    {
        // Apply the parent's gravity multiple times to overpower the sun
        ApplyGravity(planet, planets[planet.parentIndex], dt);

    }

    planet.position.x += planet.velocity.x * dt;
    planet.position.y += planet.velocity.y * dt;
    planet.trail.push_back(planet.position);
    /*if (!planet.isMoon)
    {
        planet.trail.push_back(planet.position);
    }*/
}
}

void OrbitalWorld::Update(float dt)
{   
    HandleInput();
    UpdatePhysics(dt);

}

void OrbitalWorld::Draw()
{
   ClearBackground(BLACK);

    DrawTrails();
    DrawBodies();
    DrawDragPreview();
    DrawUI();
}

void OrbitalWorld::ApplyGravity(
    OrbitalBody& body,
    const OrbitalBody& attractor,
    float dt)
{
    const float G = 500.0f;

    float dx = attractor.position.x - body.position.x;
    float dy = attractor.position.y - body.position.y;

    float distanceSquared = dx * dx + dy * dy;
    float distance = sqrt(distanceSquared);

    if (distance < 1.0f)
    {
        return;
    }

    float directionX = dx / distance;
    float directionY = dy / distance;

    float acceleration = G * attractor.mass / distanceSquared;

    body.velocity.x += directionX * acceleration * dt;
    body.velocity.y += directionY * acceleration * dt;
}

Vector2 OrbitalWorld::CalculateLaunchVelocity() const
{
    float launchScale = 0.5f;

    Vector2 launchVelocity;
    launchVelocity.x = (dragStart.x - dragCurrent.x) * launchScale;
    launchVelocity.y = (dragStart.y - dragCurrent.y) * launchScale;

    return launchVelocity;
}

void OrbitalWorld::SpawnPlanet(Vector2 position, Vector2 velocity)
{
    OrbitalBody planet;

    planet.position = position;
    planet.velocity = velocity;

    planet.radius = spawnRadius;
    planet.mass = planet.radius * planet.radius * 100.0f;
    int colorChoice = GetRandomValue(0, 4);

    if (colorChoice == 0)
    {
        planet.color = BLUE;
    }
    else if (colorChoice == 1)
    {
        planet.color = SKYBLUE;
    }
    else if (colorChoice == 2)
    {
        planet.color = GREEN;
    }
    else if (colorChoice == 3)
    {
        planet.color = ORANGE;
    }
    else
    {
        planet.color = PURPLE;
    }

    planets.push_back(planet);
}

void OrbitalWorld::DrawTrails()
{
    for (const OrbitalBody& planet : planets)
    {
        if (planet.isMoon)
        {
            continue;
        }

        for (int i = 1; i < planet.trail.size(); i++)
        {
            DrawLineV(planet.trail[i - 1],
                      planet.trail[i],
                      planet.color);
        }
    }
}

void OrbitalWorld::DrawBodies()
{
    DrawCircleV(sun.position, sun.radius, sun.color);

    for (int i = 0; i < planets.size(); i++)
{
    const OrbitalBody& planet = planets[i];

    DrawCircleV(planet.position,
                planet.radius,
                planet.color);

    if (i == selectedPlanetIndex)
    {
        DrawCircleLines(
            planet.position.x,
            planet.position.y,
            planet.radius + 3,
            WHITE);
    }
}
}

void OrbitalWorld::DrawDragPreview()
{
    if (!isDragging)
    {
        return;
    }

    Vector2 launchVelocity = CalculateLaunchVelocity();

    Vector2 arrowEnd;
    arrowEnd.x = dragStart.x + launchVelocity.x;
    arrowEnd.y = dragStart.y + launchVelocity.y;

    DrawLineEx(dragStart, arrowEnd, 3.0f, GREEN);
    DrawCircleV(dragStart, 5, WHITE);
    DrawCircleV(arrowEnd, 5, GREEN);
}

void OrbitalWorld::DrawUI()
{
    DrawText("Left click + drag: manual launch", 20, 60, 20, GRAY);
    DrawText("Q: stable orbit", 20, 90, 20, GRAY);
    DrawText("R: Reset", 20, 120, 20, GRAY);
    DrawText("[: decrease radius, ]: increase radius", 20, 150, 20, GRAY);
   

    DrawSelectedPlanetInfo();
    DrawText("Right click: open planet info", 20, 180, 20, GRAY);
    /* DrawText("M: spawn moon on selected planet", 20, 210, 20, GRAY); */
}

void OrbitalWorld::SelectPlanetAtMouse()

{
    selectedPlanetIndex = -1;

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < planets.size(); i++)
    {
        float dx = mouse.x - planets[i].position.x;
        float dy = mouse.y - planets[i].position.y;

        float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared <= planets[i].radius * planets[i].radius)
        {
            selectedPlanetIndex = i;
            return;
        }
    }
}

void OrbitalWorld::DrawSelectedPlanetInfo()
{
    if (selectedPlanetIndex == -1)
    {
        return;
    }

    const OrbitalBody& planet = planets[selectedPlanetIndex];

    float dx = planet.position.x - sun.position.x;
    float dy = planet.position.y - sun.position.y;

    float distance = sqrt(dx * dx + dy * dy);

    float speed = sqrt(
        planet.velocity.x * planet.velocity.x +
        planet.velocity.y * planet.velocity.y);

    DrawRectangle(560, 20, 220, 140, Fade(DARKGRAY, 0.8f));

    DrawText("Planet Info", 575, 30, 22, WHITE);

    DrawText(
        TextFormat("Speed: %.1f", speed),
        575, 60, 20, WHITE);

    DrawText(
        TextFormat("Distance: %.1f", distance),
        575, 85, 20, WHITE);

    DrawText(
        TextFormat("Mass: %.1f", planet.mass),
        575, 110, 20, WHITE);

    DrawText(
        TextFormat("Radius: %.1f", planet.radius),
        575, 135, 20, WHITE);
}

/*void OrbitalWorld::SpawnMoon()
{
    if (selectedPlanetIndex == -1)
    {
        return;
    }

        OrbitalBody& parent = planets[selectedPlanetIndex];

        OrbitalBody moon;

        float moonDistance = parent.radius + 80.0f;

        moon.position = {
            parent.position.x + moonDistance,
            parent.position.y
    };

    moon.radius = 5.0f;
    moon.mass = 2.0f;
    moon.color = LIGHTGRAY;

    moon.isMoon = true;
    moon.parentIndex = selectedPlanetIndex;

    float G = 500.0f;

    float orbitSpeed = sqrt((G * parent.mass) / moonDistance) * 1.0f;

    float parentSpeed = sqrt(
    parent.velocity.x * parent.velocity.x +
    parent.velocity.y * parent.velocity.y
    );

    Vector2 tangent;

    if (parentSpeed > 0.0f)
    {
        tangent.x = parent.velocity.x / parentSpeed;
        tangent.y = parent.velocity.y / parentSpeed;
    }
    else
    {
        tangent = { 0.0f, 1.0f };
    }

        moon.velocity = {
        parent.velocity.x + tangent.x * orbitSpeed,
        parent.velocity.y + tangent.y * orbitSpeed
    };

    planets.push_back(moon);
} */