//Handles input and rendering
#include "raylib.h"
#include <vector>
#include <cmath>
#include "RigidBody.h"
#include "PhysicsWorld.h"
#include "Physics.h"
#include "OrbitalWorld.h"
#include "FluidWorld.h"
#include "UI.h"
#include "WindWorld.h"


enum class AppState
{
    MainMenu,
    CollisionSim, 
    OrbitalSim, 
    FluidSim,
    WindSim
};

void DrawRigidBody(const RigidBody& body, Color color)
{
    DrawCircle(body.position.x, body.position.y, body.radius, color);

}
bool IsMouseOverRect(float x, float y, float width, float height)
{
    Vector2 mouse = GetMousePosition();

    return mouse.x >= x &&
           mouse.x <= x + width &&
           mouse.y >= y &&
           mouse.y <= y + height;
}
bool IsMouseOverMassSlider();
bool IsMouseOverUI()
{
    float buttonX = 670;
    float buttonY = 95;
    float buttonWidth = 120;
    float buttonHeight = 32;
    float buttonGap = 8;

    return IsMouseOverMassSlider() ||
           IsMouseOverRect(buttonX, buttonY, buttonWidth, buttonHeight) ||
           IsMouseOverRect(buttonX, buttonY + buttonHeight + buttonGap, buttonWidth, buttonHeight);
}


enum SpawnMode
{
    SPAWN_DYNAMIC,
    SPAWN_STATIC
};

void SpawnBodyAtMouse(PhysicsWorld& world, SpawnMode mode, float radius)
{
    Vector2 mousePosition = GetMousePosition();

    RigidBody newBody = CreateRigidBody(
        mousePosition.x,
        mousePosition.y,
        radius,
        1
    );

    if (mode == SPAWN_STATIC)
    {
        newBody.isStatic = true;
        newBody.velocity.x = 0;
        newBody.velocity.y = 0;
    }
    else
    {
        newBody.velocity.x = GetRandomValue(-200, 200);
        newBody.velocity.y = GetRandomValue(-200, 0);
    }

    world.AddBody(newBody);
}

void SpawnBodyWithVelocity(PhysicsWorld& world, SpawnMode mode, float radius, Vector2 position, Vector2 velocity)
{
    RigidBody newBody = CreateRigidBody(
        position.x,
        position.y,
        radius,
        1
    );

    if (mode == SPAWN_STATIC)
    {
        newBody.isStatic = true;
        newBody.velocity.x = 0;
        newBody.velocity.y = 0;
    }
    else
    {
        newBody.velocity.x = velocity.x;
        newBody.velocity.y = velocity.y;
    }

    world.AddBody(newBody);
}

int GetBodyIndexAtMouse(PhysicsWorld& world)
{
    Vector2 mouse = GetMousePosition();

    for (int i = world.GetBodyCount() - 1; i >= 0; i--)
    {
        RigidBody& body = world.GetBody(i);

        float dx = mouse.x - body.position.x;
        float dy = mouse.y - body.position.y;
        float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared <= body.radius * body.radius)
        {
            return i;
        }
    }

    return -1;
}

void DrawSelectedBodyOutline(PhysicsWorld& world, int selectedBodyIndex)
{
    if (selectedBodyIndex < 0 || selectedBodyIndex >= world.GetBodyCount())
    {
        return;
    }

    RigidBody& body = world.GetBody(selectedBodyIndex);

    DrawCircleLines(
        body.position.x,
        body.position.y,
        body.radius + 4,
        BLUE);

    DrawCircleLines(
        body.position.x,
        body.position.y,
        body.radius + 6,
        SKYBLUE);
}

void DrawVelocityVectors(PhysicsWorld& world)
{
    for (int i = 0; i < world.GetBodyCount(); i++)
    {
        RigidBody& body = world.GetBody(i);

        if (body.isStatic)
        {
            continue;
        }

        float scale = 0.15f;

        Vector2 start = {
            body.position.x,
            body.position.y
        };

        Vector2 end = {
            body.position.x + body.velocity.x * scale,
            body.position.y + body.velocity.y * scale
        };

        DrawLineEx(start, end, 2.0f, RED);

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

            DrawLineEx(end, left, 2.0f, RED);
            DrawLineEx(end, right, 2.0f, RED);
        }
    }
}

void DrawSpawnTrajectoryPreview(Vector2 start, Vector2 current, float radius)
{
    DrawCircleLines(start.x, start.y, radius, DARKGRAY);

    DrawLineEx(current, start, 2.0f, DARKGRAY);

    float dx = start.x - current.x;
    float dy = start.y - current.y;

    Vector2 previewVelocity = {
        dx * 3.0f,
        dy * 3.0f
    };

    Vector2 arrowEnd = {
        start.x + previewVelocity.x * 0.15f,
        start.y + previewVelocity.y * 0.15f
    };

    DrawLineEx(start, arrowEnd, 3.0f, BLUE);
}

void HandlePlayerInput(PhysicsWorld& world, float dt)
{
    if (world.GetBodyCount() == 0)
    {
        return;
    }

    RigidBody& player = world.GetBody(0);

    float moveForce = 2000.0f;

    if (IsKeyDown(KEY_RIGHT))
    {
       player.velocity.x += moveForce * dt;
    }

    if (IsKeyDown(KEY_LEFT))
    {
       player.velocity.x -= moveForce * dt; 
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        float jumpImpulse = 200.0f;
        ApplyImpulse(player, Vector2D{0.0f, -jumpImpulse * player.mass});
        player.isOnGround = false;
    }
}

void DrawUIOverlay(const PhysicsWorld& world, SpawnMode spawnMode, float spawnRadius, bool simulationPaused)
{   
    UILabelValue("FPS:", TextFormat("%i", GetFPS()), 10, 10);
    DrawLine(10, 45, 130, 45, LIGHTGRAY);

    UILabelValue("Bodies:", TextFormat("%i", world.GetBodyCount()), 10, 75);

    UIText("Mode:", 10, 95, 16, DARKBLUE);

    if (spawnMode == SPAWN_DYNAMIC)
    {
        UIText("Dynamic", 70, 95, 16, BLACK);
    }
    else
    {
        UIText("Static", 70, 95, 16, BLACK);
    }

    UILabelValue("Radius:", TextFormat("%.0f", spawnRadius), 10, 120);
    DrawLine(10, 160, 130, 160, LIGHTGRAY);

    
    UIText("[: Smaller   ]: Larger", 10, 190, 16, DARKBLUE);
    UIText("Left Click: Spawn", 10, 215, 16, DARKBLUE);
    UIText("Space: Jump | R: Reset", 10, 240, 16, DARKBLUE);
    UIText("V: Velocity Vectors", 10, 265, 16, DARKBLUE);
    UIText("D: Remove selected", 10, 290, 16, DARKBLUE);
    UIText("P: Pause", 10, 315, 16, DARKBLUE);
    UIText("Period (.): Step Frame", 10, 340, 16, DARKBLUE);    

    if (simulationPaused)
    {
        UIText("Status: PAUSED", 10, 365, 16, ORANGE);
    }
    else
    {
        UIText("Status: RUNNING", 10, 365, 16, DARKGREEN);
    }
}

void DrawSpawnModeButtons(SpawnMode& spawnMode)
{
    float x = 670;
    float y = 95;
    float width = 120;
    float height = 32;
    float gap = 8;

    UIText("Spawn", x, y - 35, 18, DARKBLUE);
    DrawLine(x, y - 12, x + 120, y - 12, LIGHTGRAY);

    Rectangle dynamicButton = { x, y, width, height };
    Rectangle staticButton = { x, y + height + gap, width, height };

    bool mouseOverDynamic = CheckCollisionPointRec(GetMousePosition(), dynamicButton);
    bool mouseOverStatic = CheckCollisionPointRec(GetMousePosition(), staticButton);

    Color dynamicColor = LIGHTGRAY;
    Color staticColor = LIGHTGRAY;

    if (spawnMode == SPAWN_DYNAMIC)
    {
        dynamicColor = SKYBLUE;
    }

    if (spawnMode == SPAWN_STATIC)
    {
        staticColor = SKYBLUE;
    }

    DrawRectangleRec(dynamicButton, dynamicColor);
    DrawRectangleRec(staticButton, staticColor);

    DrawRectangleLinesEx(dynamicButton, 1, DARKGRAY);
    DrawRectangleLinesEx(staticButton, 1, DARKGRAY);

    UIText("Dynamic", x + 32, y + 9, 18, BLACK);
    UIText("Static", x + 45, y + height + gap + 9, 18, BLACK);

    if (mouseOverDynamic && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        spawnMode = SPAWN_DYNAMIC;
    }

    if (mouseOverStatic && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        spawnMode = SPAWN_STATIC;
    }
}

void DrawSelectedBodyPanel(PhysicsWorld& world, int selectedBodyIndex)
{
    float x = 670;
    float y = 185;

    UIText("Selected", x, y, 18, DARKBLUE);
    DrawLine(x, y + 24, x + 120, y + 24, LIGHTGRAY);

    if (selectedBodyIndex < 0 || selectedBodyIndex >= world.GetBodyCount())
    {
        UIText("None", x, y + 45, 16, BLACK);
        return;
    }

    RigidBody& body = world.GetBody(selectedBodyIndex);

    UIText(TextFormat("Index: %i", selectedBodyIndex), x, y + 45, 14, DARKGRAY);

    if (body.isStatic)
    {
        UIText("Type: Static", x, y + 68, 14, BLACK);
    }
    else
    {
        UIText("Type: Dynamic", x, y + 68, 14, BLACK);
    }

    UIText(TextFormat("Mass: %.2f   Radius: %.0f", body.mass, body.radius), x, y + 91, 14, BLACK);
    
    UIText(
        TextFormat("Pos: (%.0f, %.0f)",
                body.position.x,
                body.position.y),
        x,
        y + 127,
        14,
        BLACK);

    UIText(
        TextFormat("Vel: (%.1f, %.1f)",
                body.velocity.x,
                body.velocity.y),
        x,
        y + 150,
        14,
        BLACK);
}

void DrawMassSlider(PhysicsWorld& world)
{
    if (world.GetBodyCount() == 0)
    {
        return;
    }

    RigidBody& player = world.GetBody(0);

    float sliderX = 670;
    float sliderY = 35;
    float sliderWidth = 120;
    float sliderHeight = 6;

    float minMass = 0.5f;
    float maxMass = 10.0f;

    Vector2 mouse = GetMousePosition();

    bool mouseOverSlider =
        mouse.x >= sliderX &&
        mouse.x <= sliderX + sliderWidth &&
        mouse.y >= sliderY - 10 &&
        mouse.y <= sliderY + sliderHeight + 10;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && mouseOverSlider)
    {
        float mouseT = (mouse.x - sliderX) / sliderWidth;

        if (mouseT < 0)
        {
            mouseT = 0;
        }

        if (mouseT > 1)
        {
            mouseT = 1;
        }

        player.mass = minMass + mouseT * (maxMass - minMass);
    }

    float t = (player.mass - minMass) / (maxMass - minMass);
    float knobX = sliderX + t * sliderWidth;

    UIText(TextFormat("Mass: %.2f", player.mass), 670, sliderY - 30, 18, DARKBLUE);

    DrawRectangle(sliderX, sliderY, sliderWidth, sliderHeight, LIGHTGRAY);
    DrawCircle(knobX, sliderY + sliderHeight / 2, 7, DARKGRAY);
}

bool IsMouseOverMassSlider()
{
    float sliderX = 670;
    float sliderY = 35;
    float sliderWidth = 120;
    float sliderHeight = 6;

    Vector2 mouse = GetMousePosition();

    return mouse.x >= sliderX &&
           mouse.x <= sliderX + sliderWidth &&
           mouse.y >= sliderY - 10 &&
           mouse.y <= sliderY + sliderHeight + 10;
}

void DrawMenuCard(Rectangle card, Color accent)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, card);

    Color fill = hover ? Fade(accent, 0.10f) : Fade(WHITE, 0.85f);

    DrawRectangleRounded(card, 0.04f, 12, fill);
    DrawRectangleRoundedLines(card, 0.04f, 12, accent);

 
    DrawLine(card.x + 25, card.y + 95, card.x + 65, card.y + 95, accent);
}

std::vector<Vector2> menuFluidParticles;

void InitMenuFluidParticles()
{
    menuFluidParticles.clear();

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 18; col++)
        {
            Vector2 particle;

            particle.x = 560 + col * 10 + GetRandomValue(-3, 3);
            particle.y = 400 + row * 10 + GetRandomValue(-3, 3);

            menuFluidParticles.push_back(particle);
        }
    }
}

int main()
{
    AppState currentState = AppState::MainMenu;
    int screenWidth = 1050;
    int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Physics Engine");
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();

    uiFont = LoadFontEx("resources/fonts/InterVariable.ttf", 24, nullptr, 0);
    InitMenuFluidParticles();

    PhysicsWorld world;
    world.screenWidth = screenWidth;
    world.screenHeight = screenHeight;
    world.gravity = 400.0f;
    world.Reset();

    OrbitalWorld orbitalWorld;
    orbitalWorld.screenWidth = screenWidth;
    orbitalWorld.screenHeight = screenHeight;
    orbitalWorld.Reset();

    FluidWorld fluidWorld;
    fluidWorld.screenWidth = screenWidth;
    fluidWorld.screenHeight = screenHeight;
    fluidWorld.Reset();

    WindWorld windWorld;
    windWorld.screenWidth = screenWidth;
    windWorld.screenHeight = screenHeight;
    windWorld.Reset();

    const float fixedDt = 1.0f / 60.0f;
    float accumulator = 0.0f;

    SpawnMode spawnMode = SPAWN_DYNAMIC;
    float spawnRadius = 20.0f;
    bool isAimingSpawn = false;
    Vector2 spawnStart = {0, 0};
    Vector2 spawnCurrent = {0, 0};
    int selectedBodyIndex = -1;
    bool showVelocityVectors = false;

    bool simulationPaused = false;

    int draggedBodyIndex = -1;
    Vector2 dragOffset = {0, 0};
    Vector2 previousMousePosition = {0, 0};

    while (!WindowShouldClose())
    {
        float frameTime = GetFrameTime();

        BeginDrawing();

        if (currentState == AppState::MainMenu)
        {
            ClearBackground(Color{ 245, 247, 250, 255 });

            //========================
            // Header
            //========================

            UIText(
                "PHYSICS SANDBOX",
                342,
                25,
                42,
                BLACK
            );

            UIText(
                "Choose a simulation to begin",
                395,
                73,
                18,
                GRAY
            );

            //========================
            // Card rectangles
            //========================

            Rectangle windButton =
            {
                105.0f,
                110.0f,
                840.0f,
                230.0f
            };

            Rectangle collisionButton =
            {
                105.0f,
                365.0f,
                260.0f,
                175.0f
            };

            Rectangle orbitalButton =
            {
                395.0f,
                365.0f,
                260.0f,
                175.0f
            };

            Rectangle fluidButton =
            {
                685.0f,
                365.0f,
                260.0f,
                175.0f
            };

            Rectangle enterWindButton =
            {
                135.0f,
                275.0f,
                190.0f,
                42.0f
            };

            Vector2 mouse = GetMousePosition();

            bool windHovered =
                CheckCollisionPointRec(mouse, windButton);

            bool collisionHovered =
                CheckCollisionPointRec(mouse, collisionButton);

            bool orbitalHovered =
                CheckCollisionPointRec(mouse, orbitalButton);

            bool fluidHovered =
                CheckCollisionPointRec(mouse, fluidButton);

            Color windFill =
                windHovered
                ? Fade(RED, 0.08f)
                : WHITE;

            Color collisionFill =
                collisionHovered
                ? Fade(BLUE, 0.08f)
                : WHITE;

            Color orbitalFill =
                orbitalHovered
                ? Fade(GREEN, 0.08f)
                : WHITE;

            Color fluidFill =
                fluidHovered
                ? Fade(SKYBLUE, 0.10f)
                : WHITE;

            //========================
            // Draw card backgrounds
            //========================

            DrawRectangleRounded(
                windButton,
                0.035f,
                12,
                windFill
            );

            DrawRectangleRoundedLines(
                windButton,
                0.035f,
                12,
                RED
            );

            DrawRectangleRounded(
                collisionButton,
                0.045f,
                12,
                collisionFill
            );

            DrawRectangleRoundedLines(
                collisionButton,
                0.045f,
                12,
                BLUE
            );

            DrawRectangleRounded(
                orbitalButton,
                0.045f,
                12,
                orbitalFill
            );

            DrawRectangleRoundedLines(
                orbitalButton,
                0.045f,
                12,
                GREEN
            );

            DrawRectangleRounded(
                fluidButton,
                0.045f,
                12,
                fluidFill
            );

            DrawRectangleRoundedLines(
                fluidButton,
                0.045f,
                12,
                SKYBLUE
            );

            //========================
            // Featured wind card
            //========================

            Rectangle featuredBadge =
            {
                130.0f,
                130.0f,
                115.0f,
                28.0f
            };

            DrawRectangleRounded(
                featuredBadge,
                0.25f,
                8,
                RED
            );

            UIText(
                "* FEATURED",
                143,
                136,
                15,
                WHITE
            );

            UIText(
                "WIND SIM",
                135,
                175,
                32,
                RED
            );

            UIText(
                "AERODYNAMIC WIND TUNNEL",
                135,
                212,
                17,
                DARKGRAY
            );

            UIText(
                "Airfoils | Flowfields | Wake Turbulence",
                135,
                242,
                15,
                GRAY
            );

            

            //========================
            // Wind streamlines
            //========================

            Vector2 windObstacle =
            {
                690.0f,
                225.0f
            };

            float windObstacleRadius = 27.0f;

            float streamlineY[] =
            {
                145.0f,
                165.0f,
                185.0f,
                205.0f,
                245.0f,
                265.0f,
                285.0f,
                305.0f
            };

            for (float baseY : streamlineY)
            {
                Vector2 previousPoint =
                {
                    430.0f,
                    baseY
                };

                for (int pointIndex = 1;
                    pointIndex <= 55;
                    pointIndex++)
                {
                    float x =
                        430.0f +
                        pointIndex * 8.5f;

                    float dx =
                        x - windObstacle.x;

                    float verticalDistance =
                        fabsf(
                            baseY -
                            windObstacle.y
                        );

                    float horizontalInfluence =
                        expf(
                            -(dx * dx) /
                            6000.0f
                        );

                    float verticalInfluence =
                        std::clamp(
                            (95.0f - verticalDistance) /
                            95.0f,
                            0.0f,
                            1.0f
                        );

                    float bendDirection =
                        baseY < windObstacle.y
                        ? -1.0f
                        : 1.0f;

                    float bend =
                        bendDirection *
                        horizontalInfluence *
                        verticalInfluence *
                        42.0f;

                    Vector2 currentPoint =
                    {
                        x,
                        baseY + bend
                    };

                    DrawLineEx(
                        previousPoint,
                        currentPoint,
                        1.6f,
                        Fade(BLUE, 0.65f)
                    );

                    previousPoint =
                        currentPoint;
                }

                float arrowX = 905.0f;

                DrawTriangle(
                    {
                        arrowX,
                        baseY
                    },
                    {
                        arrowX - 8.0f,
                        baseY - 4.0f
                    },
                    {
                        arrowX - 8.0f,
                        baseY + 4.0f
                    },
                    BLUE
                );
            }

            DrawCircleV(
                windObstacle,
                windObstacleRadius,
                RED
            );

            DrawCircleLines(
                static_cast<int>(
                    windObstacle.x
                ),
                static_cast<int>(
                    windObstacle.y
                ),
                windObstacleRadius + 2.0f,
                Fade(DARKGRAY, 0.35f)
            );

            //========================
            // Collision card
            //========================

            UIText(
                "RIGID BODY",
                130,
                385,
                20,
                BLUE
            );

            UIText(
                "COLLISION SIM",
                130,
                410,
                20,
                BLUE
            );

            DrawCircle(
                175,
                475,
                22,
                BLUE
            );

            DrawCircle(
                290,
                475,
                22,
                RED
            );

            DrawLineEx(
                { 200.0f, 475.0f },
                { 221.0f, 475.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { 221.0f, 475.0f },
                { 215.0f, 470.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { 221.0f, 475.0f },
                { 215.0f, 480.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { 265.0f, 475.0f },
                { 244.0f, 475.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { 244.0f, 475.0f },
                { 250.0f, 470.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { 244.0f, 475.0f },
                { 250.0f, 480.0f },
                2.0f,
                BLACK
            );

            //========================
            // Orbital card
            //========================

            UIText(
                "ORBITAL SIM",
                420,
                390,
                21,
                GREEN
            );

            DrawCircle(
                525,
                475,
                24,
                YELLOW
            );

            DrawEllipseLines(
                525,
                475,
                85,
                42,
                BLUE
            );

            DrawEllipseLines(
                525,
                475,
                57,
                27,
                DARKGREEN
            );

            DrawCircle(
                595,
                438,
                9,
                BLUE
            );

            DrawCircle(
                460,
                510,
                8,
                DARKGREEN
            );

            //========================
            // Fluid card
            //========================

            UIText(
                "FLUID SIM",
                710,
                390,
                21,
                SKYBLUE
            );

            Rectangle fluidCube =
            {
                785.0f,
                455.0f,
                30.0f,
                30.0f
            };

            DrawRectanglePro(
                fluidCube,
                {
                    15.0f,
                    15.0f
                },
                -12.0f,
                ORANGE
            );

            DrawCircle(
                885,
                470,
                15,
                LIGHTGRAY
            );

            for (const Vector2& particle :
                menuFluidParticles)
            {
                Vector2 menuParticlePosition =
                {
                    particle.x + 150.0f,
                    particle.y + 45.0f
                };

                if (CheckCollisionPointRec(
                        menuParticlePosition,
                        fluidButton))
                {
                    DrawCircleV(
                        menuParticlePosition,
                        1.7f,
                        BLUE
                    );
                }
            }

            //========================
            // Footer
            //========================

            UIText(
                "Use mouse to select",
                425,
                565,
                17,
                GRAY
            );

            //========================
            // Menu input
            //========================

            if (IsMouseButtonPressed(
                    MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(
                        mouse,
                        windButton))
                {
                    currentState =
                        AppState::WindSim;
                }
                else if (
                    CheckCollisionPointRec(
                        mouse,
                        collisionButton))
                {
                    currentState =
                        AppState::CollisionSim;
                }
                else if (
                    CheckCollisionPointRec(
                        mouse,
                        orbitalButton))
                {
                    currentState =
                        AppState::OrbitalSim;
                }
                else if (
                    CheckCollisionPointRec(
                        mouse,
                        fluidButton))
                {
                    currentState =
                        AppState::FluidSim;
                }
            
            }
        }

        //Collision Sim Run
        else if (currentState == AppState::CollisionSim)
        {
            ClearBackground(RAYWHITE);

            accumulator += frameTime;

            HandlePlayerInput(world, frameTime);

            if (IsKeyPressed(KEY_ONE))
            {
                spawnMode = SPAWN_DYNAMIC;
            }

            if (IsKeyPressed(KEY_TWO))
            {
                spawnMode = SPAWN_STATIC;
            }

            if (IsKeyPressed(KEY_LEFT_BRACKET))
            {
                if (spawnRadius > 5.0f)
                {
                    spawnRadius -= 5.0f;
                }
            }

            if (IsKeyPressed(KEY_RIGHT_BRACKET))
            {
                if (spawnRadius < 100.0f)
                {
                    spawnRadius += 5.0f;
                }
            }

            if (IsKeyPressed(KEY_V))
            {
                showVelocityVectors = !showVelocityVectors;
            }

            if (IsKeyPressed(KEY_D) && selectedBodyIndex != -1)
            {
                world.RemoveBody(selectedBodyIndex);
                selectedBodyIndex = -1;
                draggedBodyIndex = -1;
            }

            if (IsKeyPressed(KEY_P))
            {
                simulationPaused = !simulationPaused;
            }

            if (simulationPaused && IsKeyPressed(KEY_PERIOD))
            {
                world.Step(fixedDt);
            }


            Vector2 mouse = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsMouseOverUI())
            {
                int clickedBodyIndex = GetBodyIndexAtMouse(world);

                if (clickedBodyIndex != -1)
                {
                    selectedBodyIndex = clickedBodyIndex;
                    draggedBodyIndex = clickedBodyIndex;

                    RigidBody& body = world.GetBody(clickedBodyIndex);

                    dragOffset.x = body.position.x - mouse.x;
                    dragOffset.y = body.position.y - mouse.y;

                    previousMousePosition = mouse;
                }
                else
                {
                    isAimingSpawn = true;
                    spawnStart = mouse;
                    spawnCurrent = mouse;
                }
            }

            if (draggedBodyIndex != -1)
            {
                RigidBody& body = world.GetBody(draggedBodyIndex);

                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    Vector2 currentMouse = GetMousePosition();

                    body.position.x = currentMouse.x + dragOffset.x;
                    body.position.y = currentMouse.y + dragOffset.y;

                    body.velocity.x = (currentMouse.x - previousMousePosition.x) / frameTime;
                    body.velocity.y = (currentMouse.y - previousMousePosition.y) / frameTime;

                    previousMousePosition = currentMouse;
                }

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                {
                    draggedBodyIndex = -1;
                }

            }

            if (isAimingSpawn && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    spawnCurrent = GetMousePosition();
                }

                if (isAimingSpawn && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                {
                    float dx = spawnStart.x - spawnCurrent.x;
                    float dy = spawnStart.y - spawnCurrent.y;
                    float dragDistance = sqrt(dx * dx + dy * dy);

                    if (dragDistance < 8.0f)
                    {
                        SpawnBodyAtMouse(world, spawnMode, spawnRadius);
                    }
                    else
                    {
                        float launchStrength = 3.0f;

                        Vector2 launchVelocity = {
                            dx * launchStrength,
                            dy * launchStrength
                        };

                        SpawnBodyWithVelocity(world, spawnMode, spawnRadius, spawnStart, launchVelocity);
                    }

                    selectedBodyIndex = world.GetBodyCount() - 1;
                    isAimingSpawn = false;
                }
            
            if (IsKeyPressed(KEY_R))
            {
                world.Reset();
                selectedBodyIndex = -1;
                draggedBodyIndex = -1;
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                currentState = AppState::MainMenu;
                draggedBodyIndex= -1;
            }

            while (accumulator >= fixedDt)
            {
                if (!simulationPaused)
                {
                    world.Step(fixedDt);
                }

                accumulator -= fixedDt;
            }

            
            world.Draw();

            if (showVelocityVectors)
            {
                DrawVelocityVectors(world);
            }

            DrawSelectedBodyOutline(world, selectedBodyIndex);
            
            if (isAimingSpawn)
            {
                DrawSpawnTrajectoryPreview(spawnStart, spawnCurrent, spawnRadius);
            }

            DrawUIOverlay(world, spawnMode, spawnRadius, simulationPaused);
            DrawMassSlider(world);
            DrawSpawnModeButtons(spawnMode);
            DrawSelectedBodyPanel(world, selectedBodyIndex);
            UIText("BACKSPACE: Menu", 10, 585, 16, DARKGRAY);
        }
        //Orbital Sim run
        else if (currentState == AppState::OrbitalSim)
        {
            orbitalWorld.Update(frameTime);
            orbitalWorld.Draw();

            if (IsKeyPressed(KEY_R))
            {
                orbitalWorld.Reset();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                currentState = AppState::MainMenu;
            }
        }
        //Fluid Sim run
        else if (currentState == AppState::FluidSim)
        {
            fluidWorld.Update(frameTime);
            fluidWorld.Draw();

            if (IsKeyPressed(KEY_R))
            {
                fluidWorld.Reset();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                currentState = AppState::MainMenu;
            }
        }
        //Wind Sim run
        else if (currentState == AppState::WindSim)
        {
            windWorld.Update(frameTime);
            windWorld.Draw();
            
            if (IsKeyPressed(KEY_R))
            {
                windWorld.Reset();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                currentState = AppState::MainMenu;
            }
        }

        EndDrawing();
    }
    UnloadFont(uiFont);
    CloseWindow();

    return 0;
}