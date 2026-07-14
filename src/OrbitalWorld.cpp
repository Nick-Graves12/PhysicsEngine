#include "OrbitalWorld.h"
#include <cmath>
#include "UI.h"

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

    GenerateStars();
   
};

void OrbitalWorld::DrawStars()
{
    for (const Star& star : stars)
    {
        DrawCircleV(star.position, star.radius, star.color);
    }
}

void OrbitalWorld::GenerateStars()
{
    stars.clear();

    for (int i = 0; i < 250; i++)
    {
        Star star;

        star.position =
        {
            (float)GetRandomValue(0, screenWidth),
            (float)GetRandomValue(0, screenHeight)
        };

        star.radius = 1.0f;

        int brightness = GetRandomValue(40, 160);

        star.color = Color{
            (unsigned char)brightness,
            (unsigned char)brightness,
            (unsigned char)brightness,
            255
        };

        stars.push_back(star);
    }
}


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

    if (IsKeyPressed(KEY_V))
    {
        showVelocityVectors = !showVelocityVectors;
    }

    if (IsKeyPressed(KEY_T))
    {
        showTrails = !showTrails;
    }

    if (IsKeyPressed(KEY_P))
    {
        simulationPaused = !simulationPaused;
    }

    if (simulationPaused && IsKeyPressed(KEY_PERIOD))
    {
        UpdatePhysics(1.0f / 60.0f);
    }

    if (IsKeyPressed(KEY_G))
    {
        usePlanetGravity = !usePlanetGravity;
    }

    if (IsKeyPressed(KEY_MINUS))
    {
        if (timeScale > 0.1f)
        {
            timeScale -= 0.1f;
        }
    }

    if (IsKeyPressed(KEY_EQUAL))
    {
        if (timeScale < 5.0f)
        {
            timeScale += 0.1f;
        }
    }

    /* if (IsKeyPressed(KEY_M))
    {
        SpawnMoon();
    } */
}

void OrbitalWorld::UpdatePhysics(float dt)
{
    // Sun pulls every planet
    for (OrbitalBody& planet : planets)
    {
        ApplyGravity(planet, sun, dt);
    }

    // Every planet pulls every other planet
    if (usePlanetGravity)
    {
        for (int i = 0; i < planets.size(); i++)
        {
            for (int j = 0; j < planets.size(); j++)
            {
                if (i == j)
                {
                    continue;
                }

                ApplyGravity(planets[i], planets[j], dt);
            }
        }
    }
    // Move planets after all gravity has been applied
    for (OrbitalBody& planet : planets)
    {
        planet.position.x += planet.velocity.x * dt;
        planet.position.y += planet.velocity.y * dt;

       planet.trail.push_back(planet.position);

        float dx = planet.position.x - planet.trailStart.x;
        float dy = planet.position.y - planet.trailStart.y;
        float distanceFromTrailStart = sqrt(dx * dx + dy * dy);

        const float orbitCloseDistance = 18.0f;

        if (distanceFromTrailStart > orbitCloseDistance * 4.0f)
        {
            planet.hasMovedAwayFromTrailStart = true;
        }

        if (planet.hasMovedAwayFromTrailStart &&
            distanceFromTrailStart < orbitCloseDistance)
        {
            planet.hasCompletedFirstOrbit = true;
        }

        const int maxTrailPoints = 900;

        if (planet.hasCompletedFirstOrbit &&
            planet.trail.size() > maxTrailPoints)
        {
            planet.trail.erase(planet.trail.begin());
        }
    }
}

void OrbitalWorld::Update(float dt)
{   
    int oldWidth = screenWidth;
    int oldHeight = screenHeight;

    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    if (screenWidth != oldWidth || screenHeight != oldHeight)
    {
        sun.position = {
        screenWidth / 2.0f,
        screenHeight / 2.0f
        };

        GenerateStars();
    }
    

    HandleInput();

    if (!simulationPaused)
    {
        UpdatePhysics(dt * timeScale);
    }
}

void OrbitalWorld::Draw()
{
    ClearBackground(Color{8, 10, 20, 255});

    DrawStars();

    if (showTrails)
    {
        DrawTrails();
    }

    DrawBodies();

    if (showVelocityVectors)
    {
        DrawVelocityVectors();
    }
    DrawOrbitPrediction();
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

    planet.trailStart = planet.position;
    planet.hasMovedAwayFromTrailStart = false;
    planet.hasCompletedFirstOrbit = false;

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

        int trailSize = planet.trail.size();

        if (trailSize < 2)
        {
            continue;
        }

        const int fadeLength = 450;

        for (int i = 1; i < trailSize; i++)
        {
            float alpha = 1.0f;

            if (planet.hasCompletedFirstOrbit && i < fadeLength)
            {
                alpha = (float)i / fadeLength;
            }

            DrawLineEx(
                planet.trail[i - 1],
                planet.trail[i],
                2.0f,
                Fade(planet.color, alpha)
            );
        }
    }
}


void OrbitalWorld::DrawBodies()
{
    float pulse = sin(GetTime() * 1.5f);

    float outerGlowRadius = sun.radius * 2.0f + pulse * 3.0f;
    float innerGlowRadius = sun.radius * 1.4f + pulse * 1.5f;

    float outerAlpha = 0.10f + pulse * 0.02f;
    float innerAlpha = 0.18f + pulse * 0.03f;

    DrawCircleV(
        sun.position,
        outerGlowRadius,
        Fade(YELLOW, outerAlpha)
    );

    DrawCircleV(
        sun.position,
        innerGlowRadius,
        Fade(ORANGE, innerAlpha)
    );

    DrawCircleV(
        sun.position,
        sun.radius,
        sun.color
    );

    for (int i = 0; i < planets.size(); i++)
    {
        const OrbitalBody& planet = planets[i];

        // Direction from planet toward sun
        float dx = sun.position.x - planet.position.x;
        float dy = sun.position.y - planet.position.y;

        float length = sqrt(dx * dx + dy * dy);

        if (length > 0.01f)
        {
            dx /= length;
            dy /= length;
        }

        // Soft glow
        DrawCircleV(
            planet.position,
            planet.radius * 1.6f,
            Fade(planet.color, 0.08f)
        );

        // Base planet
        DrawCircleV(
            planet.position,
            planet.radius,
            planet.color
        );

        // Dark side, shifted away from sun
        Vector2 shadowPosition = {
            planet.position.x - dx * planet.radius * 0.35f,
            planet.position.y - dy * planet.radius * 0.35f
        };

        DrawCircleV(
            shadowPosition,
            planet.radius * 0.9f,
            Fade(BLACK, 0.35f)
        );

        // Lit side, shifted toward sun
        Vector2 highlightPosition = {
            planet.position.x + dx * planet.radius * 0.35f,
            planet.position.y + dy * planet.radius * 0.35f
        };

        DrawCircleV(
            highlightPosition,
            planet.radius * 0.45f,
            Fade(WHITE, 0.18f)
        );

        // Subtle outline
        DrawCircleLines(
            planet.position.x,
            planet.position.y,
            planet.radius,
            Fade(WHITE, 0.25f)
        );

        if (i == selectedPlanetIndex)
        {
            DrawCircleLines(
                planet.position.x,
                planet.position.y,
                planet.radius + 3,
                WHITE
            );
        }
    }
}

void OrbitalWorld::DrawVelocityVectors()
{
    for (const OrbitalBody& planet : planets)
    {
        float scale = 0.15f;

        Vector2 start = planet.position;

        Vector2 end = {
            planet.position.x + planet.velocity.x * scale,
            planet.position.y + planet.velocity.y * scale
        };

        DrawLineEx(start, end, 2.0f, GREEN);

        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float length = sqrt(dx * dx + dy * dy);

        if (length > 0.01f)
        {
            dx /= length;
            dy /= length;

            Vector2 left = {
                end.x - dx * 10 - dy * 5,
                end.y - dy * 10 + dx * 5
            };

            Vector2 right = {
                end.x - dx * 10 + dy * 5,
                end.y - dy * 10 - dx * 5
            };

            DrawLineEx(end, left, 2.0f, GREEN);
            DrawLineEx(end, right, 2.0f, GREEN);
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

void OrbitalWorld::DrawOrbitPrediction()
{
    if (!isDragging)
    {
        return;
    }

    OrbitalBody preview;
    preview.position = dragStart;
    preview.velocity = CalculateLaunchVelocity();

    preview.radius = spawnRadius;
    preview.mass = spawnRadius * spawnRadius * 100.0f;

    float predictionDT = 1.0f / 120.0f;

    const int maxPredictionSteps = 5000;
    const float escapeDistance = 3000.0f;
    const float closeEnoughDistance = 14.0f;

    Vector2 previousPosition = preview.position;
    Vector2 startingPosition = preview.position;

    bool hasMovedAwayFromStart = false;

    for (int i = 0; i < maxPredictionSteps; i++)
    {
        ApplyGravity(preview, sun, predictionDT);

        if (usePlanetGravity)
        {
            for (const OrbitalBody& planet : planets)
            {
                ApplyGravity(preview, planet, predictionDT);
            }
        }

        preview.position.x += preview.velocity.x * predictionDT;
        preview.position.y += preview.velocity.y * predictionDT;

        if (i % 3 == 0)
        {
            DrawLineV(previousPosition, preview.position, Fade(SKYBLUE, 0.8f));
        }

        float dxStart = preview.position.x - startingPosition.x;
        float dyStart = preview.position.y - startingPosition.y;
        float distanceFromStart = sqrt(dxStart * dxStart + dyStart * dyStart);

        float dxSun = preview.position.x - sun.position.x;
        float dySun = preview.position.y - sun.position.y;
        float distanceFromSun = sqrt(dxSun * dxSun + dySun * dySun);

        if (distanceFromStart > closeEnoughDistance * 4.0f)
        {
            hasMovedAwayFromStart = true;
        }

        if (hasMovedAwayFromStart && distanceFromStart < closeEnoughDistance)
        {
            break;
        }

        if (distanceFromSun > escapeDistance)
        {
            break;
        }

        previousPosition = preview.position;
    }
}

void OrbitalWorld::DrawUI()
{   
    DrawRectangleRounded(
    {15,15,320,430},
    0.1f,
    10,
    Fade(BLACK,0.35f)
    );

    UIText("Left click + drag: Manual launch", 20, 20, 16, SKYBLUE);
    UIText("Right click: Open planet info", 20, 45, 16, SKYBLUE);
    DrawSelectedPlanetInfo();

    DrawLine(20, 70, 140, 70, WHITE);

    UIText("Q: Stable orbit", 20, 95, 16, SKYBLUE);
    UIText("R: Reset", 20, 120, 16, SKYBLUE);
    UIText(TextFormat("[: decrease radius, ]: Increase radius (%.1f)", spawnRadius), 20, 145, 16, SKYBLUE);

    DrawLine(20, 170, 140, 170, WHITE);


    UIText("V: Toggle velocity vectors", 20, 195, 16, SKYBLUE);
    UIText("T: Toggle Trials", 20, 220, 16, SKYBLUE);
    UIText("G: Toggle planet gravity", 20, 245, 16, SKYBLUE);
    UIText(usePlanetGravity ? "Planet Gravity: ON" : "Planet Gravity: OFF",
        20,
        270,
        16,
        usePlanetGravity ? GREEN : ORANGE
    );

    DrawLine(20, 295, 140, 295, WHITE);

    UIText("P: Pause", 20, 320, 16, SKYBLUE);
    UIText("Period (.): step frame", 20, 345, 16, SKYBLUE);
    if (simulationPaused)
    {
        UIText("Status: PAUSED", 20, 370, 16, ORANGE);
    }
    else
    {
        UIText("Status: RUNNING", 20, 370, 16, GREEN);
    }
    UIText(TextFormat("Time Scale: %.1fx", timeScale), 20, 395, 16, SKYBLUE);
    UIText("- / = : Adjust time scale", 20, 420, 16, SKYBLUE);

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

    DrawRectangle(560, 20, 220, 100, Fade(WHITE, 0.8f));

    UIText("Planet Info", 575, 20, 16, DARKBLUE);

    UILabelValue("Speed:", TextFormat("%.1f", speed), 575, 40);

    UILabelValue("Distance:", TextFormat("  %.1f", distance), 575, 60);

    UILabelValue("Mass:", TextFormat("%.1f", planet.mass), 575, 80);
    
    UILabelValue("Radius:", TextFormat("%.1f", planet.radius), 575, 100);
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