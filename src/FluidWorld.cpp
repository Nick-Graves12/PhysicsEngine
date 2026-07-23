#include "FluidWorld.h"
#include <cmath>
#include <algorithm>
#include "UI.h"

void FluidWorld::Reset()
{
    InitializeFluidRendering();

    tank.x = 305;
    tank.y = 80;
    tank.width = 440;
    tank.height = 480;

    InitializeSpatialGrid();

    particles.clear();

    for (int row = 0; row < 32; row++)
    {
        for (int col = 0; col < 52; col++)
        {
            float waterDepth = 260.0f;
            SpawnParticle({
                tank.x + 30.0f + col * 5.0f + GetRandomValue(-1, 1),
                tank.y + tank.height - waterDepth + row * 5.0f + GetRandomValue(-3, 3)
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
    //frame starts -> read input -> update particles -> update box -> handle collisions -> draw frame
    if (dt > 1.0f / 60.0f)
    {
        dt = 1.0f / 60.0f;
    }

    const int substeps = 1;
    float substepDT = dt / substeps;

    HandleInput();

    for (int i = 0; i < substeps; i++)
    {
        UpdateParticles(substepDT);
        UpdateBoxes(substepDT);
        ResolveParticleBoxCollisions();
    }
}

void FluidWorld::Draw()
{
    ClearBackground(Color{ 5, 8, 18, 255 });

    // Dark interior of the tank.
    DrawRectangle(
        (int)(tank.x + 4.0f),
        (int)(tank.y + 4.0f),
        (int)(tank.width - 8.0f),
        (int)(tank.height - 8.0f),
        Color{ 3, 6, 15, 255 }
    );

    // Draw floating boxes first so water can render over them.
    for (const FluidBox& box : boxes)
    {
        Rectangle rect = {
            box.position.x,
            box.position.y,
            box.width,
            box.height
        };

        Vector2 origin = {
            box.width / 2.0f,
            box.height / 2.0f
        };

        // Box shadow.
        Rectangle shadowRect = rect;
        shadowRect.x += 4.0f;
        shadowRect.y += 4.0f;

        DrawRectanglePro(
            shadowRect,
            origin,
            box.rotation,
            Fade(BLACK, 0.30f)
        );

        // Main box body.
        DrawRectanglePro(
            rect,
            origin,
            box.rotation,
            Color{ 220, 140, 20, 255 }
        );

        float halfWidth =
            box.width / 2.0f;

        float halfHeight =
            box.height / 2.0f;

        float angle =
            box.rotation * DEG2RAD;

        float cosine = cos(angle);
        float sine = sin(angle);

        Vector2 localCorners[4] = {
            { -halfWidth, -halfHeight },
            {  halfWidth, -halfHeight },
            {  halfWidth,  halfHeight },
            { -halfWidth,  halfHeight }
        };

        Vector2 worldCorners[4];

        for (int i = 0; i < 4; i++)
        {
            worldCorners[i].x =
                box.position.x +
                localCorners[i].x * cosine -
                localCorners[i].y * sine;

            worldCorners[i].y =
                box.position.y +
                localCorners[i].x * sine +
                localCorners[i].y * cosine;
        }

        // Gold rotated outline.
        for (int i = 0; i < 4; i++)
        {
            int next = (i + 1) % 4;

            DrawLineEx(
                worldCorners[i],
                worldCorners[next],
                3.0f,
                GOLD
            );
        }

        // Bright top edge.
        DrawLineEx(
            worldCorners[0],
            worldCorners[1],
            2.5f,
            Fade(WHITE, 0.35f)
        );

        // Slight highlight on the left edge.
        DrawLineEx(
            worldCorners[3],
            worldCorners[0],
            2.0f,
            Fade(GOLD, 0.35f)
        );

        // Dark bottom edge.
        DrawLineEx(
            worldCorners[2],
            worldCorners[3],
            3.0f,
            Fade(BROWN, 0.45f)
        );

        // Dark right edge.
        DrawLineEx(
            worldCorners[1],
            worldCorners[2],
            2.5f,
            Fade(BROWN, 0.30f)
        );
    }

    // Draw the metaball water over the scene.
    DrawMetaballFluid();

    // Add a translucent underwater layer over each box.
    for (const FluidBox& box : boxes)
    {
        float waterSurfaceY = tank.y + tank.height;
        float searchHalfWidth = box.width * 0.75f;

        for (const FluidParticle& particle : particles)
        {
            bool isNearBox =
                particle.position.x > box.position.x - searchHalfWidth &&
                particle.position.x < box.position.x + searchHalfWidth;

            bool isDenseWater =
                particle.density > 5.0f;

            if (isNearBox &&
                isDenseWater &&
                particle.position.y < waterSurfaceY)
            {
                waterSurfaceY = particle.position.y;
            }
        }

        Rectangle rect = {
            box.position.x,
            box.position.y,
            box.width,
            box.height
        };

        Vector2 origin = {
            box.width / 2.0f,
            box.height / 2.0f
        };

        float boxBottom =
            box.position.y + box.height / 2.0f;

        if (boxBottom > waterSurfaceY)
        {
            const float inset = 4.0f;

            BeginScissorMode(
                (int)(box.position.x - box.width),
                (int)waterSurfaceY,
                (int)(box.width * 2.0f),
                (int)(tank.y + tank.height - waterSurfaceY)
            );

            DrawRectanglePro(
                rect,
                origin,
                box.rotation,
                Fade(Color{ 0, 120, 190, 255 }, 0.12f)
            );

            EndScissorMode();
        }
    }

    // Redraw box outlines over the water.
    for (const FluidBox& box : boxes)
    {
        float halfWidth = box.width / 2.0f;
        float halfHeight = box.height / 2.0f;

        float angle = box.rotation * DEG2RAD;
        float cosine = cos(angle);
        float sine = sin(angle);

        Vector2 localCorners[4] = {
            { -halfWidth, -halfHeight },
            {  halfWidth, -halfHeight },
            {  halfWidth,  halfHeight },
            { -halfWidth,  halfHeight }
        };

        Vector2 worldCorners[4];

        for (int i = 0; i < 4; i++)
        {
            worldCorners[i].x =
                box.position.x +
                localCorners[i].x * cosine -
                localCorners[i].y * sine;

            worldCorners[i].y =
                box.position.y +
                localCorners[i].x * sine +
                localCorners[i].y * cosine;
        }

        for (int i = 0; i < 4; i++)
        {
            int next = (i + 1) % 4;

            DrawLineEx(
                worldCorners[i],
                worldCorners[next],
                2.5f,
                GOLD
            );
        }

        DrawLineEx(
            worldCorners[0],
            worldCorners[1],
            2.0f,
            Fade(WHITE, 0.35f)
        );
    }



    // Draw the tank frame over both the water and boxes.
    DrawRectangleLinesEx(
        tank,
        8.0f,
        Fade(LIGHTGRAY, 0.85f)
    );

    Rectangle innerTank = {
        tank.x + 5.0f,
        tank.y + 5.0f,
        tank.width - 10.0f,
        tank.height - 10.0f
    };

    DrawRectangleLinesEx(
        innerTank,
        1.5f,
        Fade(WHITE, 0.18f)
    );

    // Subtle glass reflections.
    DrawLineEx(
        {
            tank.x + 16.0f,
            tank.y + 18.0f
        },
        {
            tank.x + 16.0f,
            tank.y + tank.height - 18.0f
        },
        2.0f,
        Fade(WHITE, 0.09f)
    );

    DrawLineEx(
        {
            tank.x + tank.width - 18.0f,
            tank.y + 20.0f
        },
        {
            tank.x + tank.width - 18.0f,
            tank.y + 130.0f
        },
        2.0f,
        Fade(WHITE, 0.06f)
    );

    // UI.
    Rectangle controlsPanel = {
        10.0f,
        10.0f,
        250.0f,
        130.0f
    };

    DrawRectangleRounded(
        controlsPanel,
        0.06f,
        8,
        Fade(BLACK, 0.62f)
    );

    DrawRectangleRoundedLines(
        controlsPanel,
        0.06f,
        8,
        Fade(SKYBLUE, 0.28f)
    );

    UIText(
        "Fluid simulation",
        24,
        20,
        18,
        SKYBLUE
    );

    DrawLine(24, 45, 246, 45, Fade(WHITE, 0.35f));

    UIText("R: Reset", 24, 58, 16, GRAY);
    UIText("Hold left-click: Stir fluid", 24, 82, 16, GRAY);

    UIText("FPS", 24, 106, 16, GRAY);

    UIText(
        TextFormat("%i", GetFPS()),
        119,
        106,
        16,
        GREEN
    );

    UIText(
        "Backspace: Menu",
        24,
        570,
        16,
        GRAY
    );
}

void FluidWorld::HandleInput()
{
    // Stir only while holding the left mouse button.
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    Vector2 mousePosition = GetMousePosition();

    // Do nothing if the mouse is outside the tank.
    if (!CheckCollisionPointRec(mousePosition, tank))
    {
        return;
    }

    Vector2 mouseMovement = GetMouseDelta();

    // Prevent an unusually large mouse jump from exploding the fluid.
    mouseMovement.x =
        std::clamp(mouseMovement.x, -35.0f, 35.0f);

    mouseMovement.y =
        std::clamp(mouseMovement.y, -35.0f, 35.0f);

    const float stirringRadius = 55.0f;
    const float stirringRadiusSquared =
        stirringRadius * stirringRadius;

    const float stirringStrength = 8.0f;

    for (FluidParticle& particle : particles)
    {
        float dx =
            particle.position.x - mousePosition.x;

        float dy =
            particle.position.y - mousePosition.y;

        float distanceSquared =
            dx * dx + dy * dy;

        // Ignore particles outside the stirring radius.
        if (distanceSquared >= stirringRadiusSquared)
        {
            continue;
        }

        float distance = sqrt(distanceSquared);

        float closeness =
            1.0f - distance / stirringRadius;

        particle.velocity.x +=
            mouseMovement.x *
            stirringStrength *
            closeness;

        particle.velocity.y +=
            mouseMovement.y *
            stirringStrength *
            closeness;
    }
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
    // 1. Apply gravity and damping.
    for (FluidParticle& particle : particles)
    {
        particle.velocity.y += gravity * dt;

        particle.velocity.x *= 0.9998f;
        particle.velocity.y *= 0.9998f;
    }

    // 2. Build the grid using the particles' current positions.
    BuildSpatialGrid();

    // 3. Calculate density and pressure before moving particles.
    ComputeDensity();
    ApplyPressure(dt);

    // 4. Move particles using their updated velocities.
    for (FluidParticle& particle : particles)
    {
        particle.position.x += particle.velocity.x * dt;
        particle.position.y += particle.velocity.y * dt;

        ResolveWallCollisions(particle);
    }

    // 5. Correct any remaining overlaps.
    ResolveParticleCollisions();

    // 6. Keep viscosity frame-skipped for performance.

    /* viscosityFrameCounter++;

    if (viscosityFrameCounter >= 3)
    {
        ApplyViscosity();
        viscosityFrameCounter = 0;
    } */
}

void FluidWorld::InitializeFluidRendering()
{
    //If the graphics resources don't exist yet load them
    if (fluidRenderingLoaded)
    {
        return;
    }

    fluidRenderTexture = LoadRenderTexture(
        GetScreenWidth(),
        GetScreenHeight()
    );

    metaballShader = LoadShader(
        nullptr,
        "resources/shaders/metaball.fs"
    );

    fluidRenderingLoaded = true;
}

FluidWorld::~FluidWorld()
{
    if (fluidRenderingLoaded)
    {
        UnloadRenderTexture(fluidRenderTexture);
        UnloadShader(metaballShader);
    }
}

void FluidWorld::DrawMetaballFluid()
{
    BeginTextureMode(fluidRenderTexture);

    ClearBackground(BLANK);

    BeginBlendMode(BLEND_ADDITIVE);

    for (const FluidParticle& particle : particles)
    {
        float densityAmount =
            std::clamp(
                particle.density / 10.0f,
                0.0f,
                1.0f
            );

        float fieldRadius =
            particle.radius +
            7.0f +
            densityAmount * 2.5f;

        DrawCircleGradient(
            (int)particle.position.x,
            (int)particle.position.y,
            fieldRadius,
            Color{ 255, 255, 255, 130 },
            Color{ 255, 255, 255, 0 }
        );
    }

    EndBlendMode();
    EndTextureMode();

    Rectangle source = {
        0.0f,
        0.0f,
        (float)fluidRenderTexture.texture.width,
        -(float)fluidRenderTexture.texture.height
    };

    Rectangle destination = {
        0.0f,
        0.0f,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };

    // Clip only the finished shader output.
    BeginScissorMode(
        (int)(tank.x + 4.0f),
        (int)(tank.y + 4.0f),
        (int)(tank.width - 8.0f),
        (int)(tank.height - 8.0f)
    );

    BeginShaderMode(metaballShader);

    DrawTexturePro(
        fluidRenderTexture.texture,
        source,
        destination,
        { 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    EndShaderMode();
    EndScissorMode();
}   

void FluidWorld::ApplyViscosity()
{
    const float viscosityRadius = 12.0f;
    const float viscosityRadiusSquared =
        viscosityRadius * viscosityRadius;

    const float viscosityStrength = 0.0f;

    for (int i = 0; i < particles.size(); i++)
    {
        FluidParticle& a = particles[i];

        int centerCellX =
            (int)((a.position.x - tank.x) / gridCellSize);

        int centerCellY =
            (int)((a.position.y - tank.y) / gridCellSize);

        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            for (int offsetX = -1; offsetX <= 1; offsetX++)
            {
                int cellX = centerCellX + offsetX;
                int cellY = centerCellY + offsetY;

                if (cellX < 0 || cellX >= gridColumns ||
                    cellY < 0 || cellY >= gridRows)
                {
                    continue;
                }

                int gridIndex =
                    cellY * gridColumns + cellX;

                for (int j : spatialGrid[gridIndex])
                {
                    if (j <= i)
                    {
                        continue;
                    }

                    FluidParticle& b = particles[j];

                    float dx = b.position.x - a.position.x;
                    float dy = b.position.y - a.position.y;

                    float distanceSquared =
                        dx * dx + dy * dy;

                    if (distanceSquared >= viscosityRadiusSquared)
                    {
                        continue;
                    }

                    float distance = sqrt(distanceSquared);

                    float closeness =
                        1.0f - distance / viscosityRadius;

                    float velocityDifferenceX =
                        b.velocity.x - a.velocity.x;

                    float velocityDifferenceY =
                        b.velocity.y - a.velocity.y;

                    float viscosityAmount =
                        viscosityStrength * closeness;

                    float relativeSpeed =
                        sqrt(
                            velocityDifferenceX * velocityDifferenceX +
                            velocityDifferenceY * velocityDifferenceY
                        );

                    float speedFactor =
                        std::clamp(relativeSpeed / 150.0f, 0.0f, 1.0f);

                    float adjustmentX =
                        velocityDifferenceX *
                        viscosityAmount *
                        speedFactor;

                    float adjustmentY =
                        velocityDifferenceY *
                        viscosityAmount *
                        speedFactor;

                    a.velocity.x += adjustmentX;
                    a.velocity.y += adjustmentY;

                    b.velocity.x -= adjustmentX;
                    b.velocity.y -= adjustmentY;
                }
            }
        }
    }
}

void FluidWorld::ComputeDensity()
{
    const float densityRadius = 14.0f;
    const float densityRadiusSquared =
        densityRadius * densityRadius;

    for (FluidParticle& particle : particles)
    {
        particle.density = 0.0f;
    }

    for (int i = 0; i < particles.size(); i++)
    {
        FluidParticle& particle = particles[i];

        int centerCellX =
            (int)((particle.position.x - tank.x) / gridCellSize);

        int centerCellY =
            (int)((particle.position.y - tank.y) / gridCellSize);

        // Search this cell and the eight neighboring cells
        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            for (int offsetX = -1; offsetX <= 1; offsetX++)
            {
                int cellX = centerCellX + offsetX;
                int cellY = centerCellY + offsetY;

                if (cellX < 0 || cellX >= gridColumns ||
                    cellY < 0 || cellY >= gridRows)
                {
                    continue;
                }

                int gridIndex =
                    cellY * gridColumns + cellX;

                for (int neighborIndex : spatialGrid[gridIndex])
                {
                    const FluidParticle& neighbor =
                        particles[neighborIndex];

                    float dx =
                        neighbor.position.x -
                        particle.position.x;

                    float dy =
                        neighbor.position.y -
                        particle.position.y;

                    float distanceSquared =
                        dx * dx + dy * dy;

                    if (distanceSquared <
                        densityRadiusSquared)
                    {
                        float distance =
                            sqrt(distanceSquared);

                        float closeness =
                            1.0f -
                            distance / densityRadius;

                        particle.density += closeness;
                    }
                }
            }
        }
    }
}

void FluidWorld::ApplyPressure(float dt)
{
    const float restDensity = 5.0f;
    const float pressureStrength = 900.0f;
    const float pressureRadius = 14.0f;

    for (FluidParticle& particle : particles)
    {
        float densityExcess =
            particle.density - restDensity;

        if (densityExcess < 0.0f)
        {
            densityExcess = 0.0f;
        }

        particle.pressure =
            pressureStrength *
            (
                densityExcess +
                densityExcess * densityExcess * 0.8f
            );

        particle.pressure =
            std::clamp(
                particle.pressure,
                0.0f,
                12000.0f
            );
    }

    for (int i = 0; i < particles.size(); i++)
    {
        FluidParticle& particle = particles[i];

        int centerCellX =
            (int)((particle.position.x - tank.x) / gridCellSize);

        int centerCellY =
            (int)((particle.position.y - tank.y) / gridCellSize);

        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            for (int offsetX = -1; offsetX <= 1; offsetX++)
            {
                int cellX = centerCellX + offsetX;
                int cellY = centerCellY + offsetY;

                if (cellX < 0 || cellX >= gridColumns ||
                    cellY < 0 || cellY >= gridRows)
                {
                    continue;
                }

                int gridIndex =
                    cellY * gridColumns + cellX;

                for (int neighborIndex : spatialGrid[gridIndex])
                {
                    if (neighborIndex <= i)
                    {
                        continue;
                    }

                    FluidParticle& neighbor =
                        particles[neighborIndex];

                    float dx =
                        neighbor.position.x -
                        particle.position.x;

                    float dy =
                        neighbor.position.y -
                        particle.position.y;

                    float distanceSquared =
                        dx * dx + dy * dy;

                    if (distanceSquared <= 0.0001f ||
                        distanceSquared >=
                        pressureRadius * pressureRadius)
                    {
                        continue;
                    }

                    float distance =
                        sqrt(distanceSquared);

                    float normalX = dx / distance;
                    float normalY = dy / distance;

                    float closeness =
                        1.0f -
                        distance / pressureRadius;

                    float averagePressure =
                        (particle.pressure +
                        neighbor.pressure) * 0.5f;

                    float averageDensity =
                        (particle.density +
                        neighbor.density) * 0.5f;

                    averageDensity =
                        std::max(averageDensity, 1.0f);

                    float acceleration =
                        averagePressure *
                        closeness /
                        averageDensity;

                    float velocityChange =
                        acceleration * dt;

                    particle.velocity.x -=
                        normalX * velocityChange;

                    particle.velocity.y -=
                        normalY * velocityChange;

                    neighbor.velocity.x += 
                        normalX * velocityChange;

                    neighbor.velocity.y +=
                        normalY * velocityChange;
                }
            }
        }
    }
}

void FluidWorld::ResolveParticleCollisions()
{
    for (int i = 0; i < particles.size(); i++)
    {
        FluidParticle& a = particles[i];

        int centerCellX =
            (int)((a.position.x - tank.x) / gridCellSize);

        int centerCellY =
            (int)((a.position.y - tank.y) / gridCellSize);

        for (int offsetY = -1; offsetY <= 1; offsetY++)
        {
            for (int offsetX = -1; offsetX <= 1; offsetX++)
            {
                int cellX = centerCellX + offsetX;
                int cellY = centerCellY + offsetY;

                if (cellX < 0 || cellX >= gridColumns ||
                    cellY < 0 || cellY >= gridRows)
                {
                    continue;
                }

                int gridIndex =
                    cellY * gridColumns + cellX;

                for (int j : spatialGrid[gridIndex])
                {
                    if (j <= i)
                    {
                        continue;
                    }

                    FluidParticle& b = particles[j];

                    float dx = b.position.x - a.position.x;
                    float dy = b.position.y - a.position.y;

                    float distanceSquared =
                        dx * dx + dy * dy;

                    float minDistance =
                        a.radius + b.radius;

                    if (distanceSquared >=
                        minDistance * minDistance)
                    {
                        continue;
                    }

                    if (distanceSquared < 0.0001f)
                    {
                        continue;
                    }

                    float distance =
                        sqrt(distanceSquared);

                    float normalX = dx / distance;
                    float normalY = dy / distance;

                    float overlap =
                        minDistance - distance;

                    const float correctionStrength = 0.25f;

                    a.position.x -=
                        normalX *
                        overlap *
                        correctionStrength;

                    a.position.y -=
                        normalY *
                        overlap *
                        correctionStrength;

                    b.position.x +=
                        normalX *
                        overlap *
                        correctionStrength;

                    b.position.y +=
                        normalY *
                        overlap *
                        correctionStrength;

                    float relativeVelocityX =
                        b.velocity.x - a.velocity.x;

                    float relativeVelocityY =
                        b.velocity.y - a.velocity.y;

                    float velocityAlongNormal =
                        relativeVelocityX * normalX +
                        relativeVelocityY * normalY;

                    if (velocityAlongNormal < 0.0f)
                    {
                        const float particleRestitution = 0.15f;

                        float impulse =
                            -(1.0f + particleRestitution) *
                            velocityAlongNormal *
                            0.5f;

                        a.velocity.x -= normalX * impulse;
                        a.velocity.y -= normalY * impulse;

                        b.velocity.x += normalX * impulse;
                        b.velocity.y += normalY * impulse;
                    }
                }
            }
        }
    }
}

void FluidWorld::ResolveParticleBoxCollisions()
{
    const float restitution = 0.02f;

    for (FluidBox& box : boxes)
    {
        float halfWidth = box.width / 2.0f;
        float halfHeight = box.height / 2.0f;

        float angle = box.rotation * DEG2RAD;
        float cosine = cos(angle);
        float sine = sin(angle);

        for (FluidParticle& particle : particles)
        {
            // Particle position relative to the box center
            float relativeX = particle.position.x - box.position.x;
            float relativeY = particle.position.y - box.position.y;

            // Rotate the particle into the box's local coordinate system
            float localX = relativeX * cosine + relativeY * sine;
            float localY = -relativeX * sine + relativeY * cosine;

            // Find the closest point on the rectangle
            float closestX = std::clamp(localX, -halfWidth, halfWidth);
            float closestY = std::clamp(localY, -halfHeight, halfHeight);

            float differenceX = localX - closestX;
            float differenceY = localY - closestY;

            float distanceSquared =
                differenceX * differenceX +
                differenceY * differenceY;

            float particleRadius = particle.radius + 1.5f;

            if (distanceSquared >= particleRadius * particleRadius)
            {
                continue;
            }

            float normalX = 0.0f;
            float normalY = 0.0f;
            float penetration = 0.0f;

            // Particle center is inside the box
            if (distanceSquared < 0.0001f)
            {
                float distanceLeft = localX + halfWidth;
                float distanceRight = halfWidth - localX;
                float distanceTop = localY + halfHeight;
                float distanceBottom = halfHeight - localY;

                float nearestEdge = distanceLeft;
                normalX = -1.0f;
                normalY = 0.0f;

                if (distanceRight < nearestEdge)
                {
                    nearestEdge = distanceRight;
                    normalX = 1.0f;
                    normalY = 0.0f;
                }

                if (distanceTop < nearestEdge)
                {
                    nearestEdge = distanceTop;
                    normalX = 0.0f;
                    normalY = -1.0f;
                }

                if (distanceBottom < nearestEdge)
                {
                    nearestEdge = distanceBottom;
                    normalX = 0.0f;
                    normalY = 1.0f;
                }

                penetration = nearestEdge + particleRadius;
            }
            else
            {
                float distance = sqrt(distanceSquared);

                normalX = differenceX / distance;
                normalY = differenceY / distance;

                penetration = particleRadius - distance;
            }

            // Rotate the collision normal back into world coordinates
            float worldNormalX =
                normalX * cosine - normalY * sine;

            float worldNormalY =
                normalX * sine + normalY * cosine;

            // Push the particle outside the box
            particle.position.x += worldNormalX * penetration;
            particle.position.y += worldNormalY * penetration;

            // Remove velocity moving into the box
            // Particle velocity relative to the moving box.
            float relativeVelocityX =
                particle.velocity.x - box.velocity.x;

            float relativeVelocityY =
                particle.velocity.y - box.velocity.y;

            float velocityAlongNormal =
                relativeVelocityX * worldNormalX +
                relativeVelocityY * worldNormalY;

            if (velocityAlongNormal < 0.0f)
            {
                const float splashStrength = 0.65f;

                float impulse =
                    -(1.0f + restitution) *
                    velocityAlongNormal *
                    splashStrength;

                // Push the particle away using the box's impact velocity.
                particle.velocity.x +=
                    worldNormalX * impulse;

                particle.velocity.y +=
                    worldNormalY * impulse;

                // Small opposite reaction on the box.
                box.velocity.x -=
                    worldNormalX *
                    impulse *
                    0.0015f;

                box.velocity.y -=
                    worldNormalY *
                    impulse *
                    0.0015f;
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

    box.rotation = 0.0f;
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
        box.angularVelocity *= 0.995f;

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

    float borderThickness = 8.0f;
    float particleGlow = 1.5f;
    float safetyPadding = 3.0f;
    float visualRadius = particle.radius + particleGlow + borderThickness + safetyPadding;

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

            box.angularVelocity += sideOffset * 0.00008f * submersion;
        }
    }
}

void FluidWorld::ResolveBoxWallCollisions(FluidBox& box)
{
    float restitution = 0.3f;

    float halfWidth = box.width / 2.0f;
    float halfHeight = box.height / 2.0f;

    float rotationRadians = box.rotation * DEG2RAD;

    float effectiveHalfWidth =
        fabs(cos(rotationRadians)) * halfWidth +
        fabs(sin(rotationRadians)) * halfHeight;

    float effectiveHalfHeight =
        fabs(cos(rotationRadians)) * halfHeight +
        fabs(sin(rotationRadians)) * halfWidth;

    if (box.position.x - effectiveHalfWidth < tank.x)
    {
        box.position.x = tank.x + effectiveHalfWidth;
        box.velocity.x *= -restitution;
        box.angularVelocity *= 0.8f;
    }

    if (box.position.x + effectiveHalfWidth > tank.x + tank.width)
    {
        box.position.x =
            tank.x + tank.width - effectiveHalfWidth;

        box.velocity.x *= -restitution;
        box.angularVelocity *= 0.8f;
    }

    if (box.position.y - effectiveHalfHeight < tank.y)
    {
        box.position.y = tank.y + effectiveHalfHeight;
        box.velocity.y *= -restitution;
        box.angularVelocity *= 0.8f;
    }

    const float tankBorderThickness = 8.0f;
    const float interiorFloor =
        tank.y + tank.height - tankBorderThickness;

    if (box.position.y + effectiveHalfHeight > interiorFloor)
    {
        box.position.y =
            interiorFloor - effectiveHalfHeight;

        if (box.velocity.y > 0.0f)
        {
            box.velocity.y *= -restitution;
        }

        box.angularVelocity *= 0.8f;
    }
}

void FluidWorld::InitializeSpatialGrid()
{
    gridColumns = (int)ceil(tank.width / gridCellSize);
    gridRows = (int)ceil(tank.height / gridCellSize);

    spatialGrid.clear();
    spatialGrid.resize(gridColumns * gridRows);
}

int FluidWorld::GetGridIndex(Vector2 position) const
{
    int cellX = (int)((position.x - tank.x) / gridCellSize);
    int cellY = (int)((position.y - tank.y) / gridCellSize);

    if (cellX < 0 || cellX >= gridColumns ||
        cellY < 0 || cellY >= gridRows)
    {
        return -1;
    }

    return cellY * gridColumns + cellX;
}

void FluidWorld::BuildSpatialGrid()
{
    for (std::vector<int>& cell : spatialGrid)
    {
        cell.clear();
    }

    for (int i = 0; i < particles.size(); i++)
    {
        int gridIndex = GetGridIndex(particles[i].position);

        if (gridIndex != -1)
        {
            spatialGrid[gridIndex].push_back(i);
        }
    }
}
