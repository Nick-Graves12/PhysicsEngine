//Handles input and rendering
#include "raylib.h"
#include <vector>
#include <cmath>
#include <algorithm>
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
    return IsMouseOverRect(10, 5, 250, 365) ||
           IsMouseOverRect(790, 5, 250, 365);
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
        Fade(SKYBLUE, 0.40f));
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

void DrawCollisionUIPanels()
{
    Rectangle leftPanel = {
        10.0f,
        5.0f,
        250.0f,
        365.0f
    };

    Rectangle rightPanel = {
        790.0f,
        5.0f,
        250.0f,
        365.0f
    };

    Color panelFill = Color{ 250, 251, 253, 245 };
    Color panelBorder = Fade(GRAY, 0.25f);

    DrawRectangleRounded(leftPanel, 0.06f, 8, panelFill);
    DrawRectangleRoundedLines(leftPanel, 0.06f, 8, panelBorder);

    DrawRectangleRounded(rightPanel, 0.06f, 8, panelFill);
    DrawRectangleRoundedLines(rightPanel, 0.06f, 8, panelBorder);
}

void DrawUIOverlay(const PhysicsWorld& world, SpawnMode spawnMode, float& spawnRadius, bool simulationPaused)
{   
    Color shortcutColor = Color{ 70, 100, 130, 255 };

    UILabelValue("FPS:", TextFormat("%i", GetFPS()), 24, 10);
    DrawLine(24, 45, 246, 45, LIGHTGRAY);

    UILabelValue("Bodies:", TextFormat("%i", world.GetBodyCount()), 24, 75);

    UIText("Mode:", 24, 95, 16, DARKBLUE);

    if (spawnMode == SPAWN_DYNAMIC)
    {
        UIText("Dynamic", 84, 95, 16, BLACK);
    }
    else
    {
        UIText("Static", 84, 95, 16, BLACK);
    }

    UILabelValue("Radius:", TextFormat("%.0f", spawnRadius), 24, 120);

    Rectangle radiusSlider = {
        24.0f,
        150.0f,
        222.0f,
        6.0f
    };

    Rectangle radiusSliderHitbox = {
        radiusSlider.x,
        radiusSlider.y - 8.0f,
        radiusSlider.width,
        radiusSlider.height + 16.0f
    };

    bool radiusSliderHovered = CheckCollisionPointRec(
        GetMousePosition(),
        radiusSliderHitbox
    );

    if (radiusSliderHovered &&
        IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float sliderAmount =
            (GetMousePosition().x - radiusSlider.x) /
            radiusSlider.width;

        sliderAmount = std::clamp(sliderAmount, 0.0f, 1.0f);
        spawnRadius = roundf(5.0f + sliderAmount * 95.0f);
    }

    float radiusAmount = (spawnRadius - 5.0f) / 95.0f;
    float radiusKnobX =
        radiusSlider.x + radiusAmount * radiusSlider.width;

    DrawRectangleRec(radiusSlider, LIGHTGRAY);
    DrawRectangle(
        radiusSlider.x,
        radiusSlider.y,
        radiusKnobX - radiusSlider.x,
        radiusSlider.height,
        Fade(SKYBLUE, 0.75f)
    );

    Color radiusKnobColor =
        radiusSliderHovered ? GRAY : DARKGRAY;

    float radiusKnobSize =
        radiusSliderHovered ? 8.0f : 7.0f;

    if (radiusSliderHovered &&
        IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        radiusKnobColor = SKYBLUE;
    }

    DrawCircle(
        radiusKnobX,
        radiusSlider.y + radiusSlider.height / 2.0f,
        radiusKnobSize,
        radiusKnobColor
    );

    DrawLine(24, 175, 246, 175, LIGHTGRAY);

    UIText("Left click: Spawn", 24, 195, 16, shortcutColor);
    UIText("Space: Jump | R: Reset", 24, 220, 16, shortcutColor);
    UIText("V: Velocity vectors", 24, 245, 16, shortcutColor);
    UIText("D: Remove selected body", 24, 270, 16, shortcutColor);
    UIText("P: Pause", 24, 295, 16, shortcutColor);
    UIText("Period (.): Step frame", 24, 320, 16, shortcutColor);

    if (simulationPaused)
    {
        UIText("Status: PAUSED", 24, 345, 16, ORANGE);
    }
    else
    {
        UIText("Status: RUNNING", 24, 345, 16, DARKGREEN);
    }
}

void DrawSpawnModeButtons(SpawnMode& spawnMode)
{
    float x = 804;
    float y = 95;
    float width = 222;
    float height = 32;
    float gap = 8;

    UIText("Spawn", x, y - 35, 18, DARKBLUE);
    DrawLine(x, y - 12, x + width, y - 12, LIGHTGRAY);

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

    if (mouseOverDynamic)
    {
        dynamicColor = spawnMode == SPAWN_DYNAMIC
            ? Color{ 120, 205, 245, 255 }
            : Color{ 220, 235, 245, 255 };

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            dynamicColor = Color{ 100, 175, 215, 255 };
        }
    }

    if (mouseOverStatic)
    {
        staticColor = spawnMode == SPAWN_STATIC
            ? Color{ 120, 205, 245, 255 }
            : Color{ 220, 235, 245, 255 };

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            staticColor = Color{ 100, 175, 215, 255 };
        }
    }

    DrawRectangleRec(dynamicButton, dynamicColor);
    DrawRectangleRec(staticButton, staticColor);

    DrawRectangleLinesEx(dynamicButton, 1, DARKGRAY);
    DrawRectangleLinesEx(staticButton, 1, DARKGRAY);

    float dynamicTextWidth = MeasureTextEx(
        uiFont,
        "Dynamic",
        18,
        1.0f
    ).x;

    float staticTextWidth = MeasureTextEx(
        uiFont,
        "Static",
        18,
        1.0f
    ).x;

    UIText(
        "Dynamic",
        x + (width - dynamicTextWidth) / 2.0f,
        y + 9,
        18,
        BLACK
    );

    UIText(
        "Static",
        x + (width - staticTextWidth) / 2.0f,
        y + height + gap + 9,
        18,
        BLACK
    );

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
    float x = 804;
    float y = 185;
    float valueX = x + 95.0f;

    UIText("Selected", x, y, 18, DARKBLUE);
    DrawLine(x, y + 24, x + 222, y + 24, LIGHTGRAY);

    if (selectedBodyIndex < 0 || selectedBodyIndex >= world.GetBodyCount())
    {
        UIText("None", x, y + 45, 16, BLACK);
        return;
    }

    RigidBody& body = world.GetBody(selectedBodyIndex);

    UIText("Index", x, y + 45, 14, DARKGRAY);
    UIText(TextFormat("%i", selectedBodyIndex), valueX, y + 45, 14, BLACK);

    if (body.isStatic)
    {
        UIText("Type", x, y + 68, 14, DARKGRAY);
        UIText("Static", valueX, y + 68, 14, BLACK);
    }
    else
    {
        UIText("Type", x, y + 68, 14, DARKGRAY);
        UIText("Dynamic", valueX, y + 68, 14, BLACK);
    }

    UIText("Mass", x, y + 91, 14, DARKGRAY);
    UIText(TextFormat("%.2f", body.mass), valueX, y + 91, 14, BLACK);

    UIText("Radius", x, y + 114, 14, DARKGRAY);
    UIText(TextFormat("%.0f", body.radius), valueX, y + 114, 14, BLACK);

    UIText("Position", x, y + 137, 14, DARKGRAY);
    UIText(
        TextFormat("(%.0f, %.0f)", body.position.x, body.position.y),
        valueX,
        y + 137,
        14,
        BLACK
    );

    UIText("Velocity", x, y + 160, 14, DARKGRAY);
    UIText(
        TextFormat("(%.1f, %.1f)", body.velocity.x, body.velocity.y),
        valueX,
        y + 160,
        14,
        BLACK
    );
}

void DrawMassSlider(PhysicsWorld& world)
{
    if (world.GetBodyCount() == 0)
    {
        return;
    }

    RigidBody& player = world.GetBody(0);

    float sliderX = 804;
    float sliderY = 35;
    float sliderWidth = 222;
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

    UIText(TextFormat("Mass: %.2f", player.mass), sliderX, sliderY - 30, 18, DARKBLUE);

    DrawRectangle(sliderX, sliderY, sliderWidth, sliderHeight, LIGHTGRAY);

    DrawRectangle(
        sliderX,
        sliderY,
        knobX - sliderX,
        sliderHeight,
        Fade(SKYBLUE, 0.75f)
    );

    Color knobColor = DARKGRAY;
    float knobRadius = 7.0f;

    if (mouseOverSlider)
    {
        knobColor = GRAY;
        knobRadius = 8.0f;
    }

    if (mouseOverSlider && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        knobColor = SKYBLUE;
    }

    DrawCircle(
        knobX,
        sliderY + sliderHeight / 2,
        knobRadius,
        knobColor
    );
}

bool IsMouseOverMassSlider()
{
    float sliderX = 804;
    float sliderY = 35;
    float sliderWidth = 222;
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
    float menuEntranceTime = 0.0f;
    float menuAnimationTime = 0.0f;

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
            Color menuBackground = Color{ 245, 247, 250, 255 };
            ClearBackground(menuBackground);

            menuEntranceTime += frameTime;
            menuAnimationTime += frameTime;

            for (int gridY = 25;
                gridY < screenHeight;
                gridY += 50)
            {
                for (int gridX = 25;
                    gridX < screenWidth;
                    gridX += 50)
                {
                    DrawCircle(
                        gridX,
                        gridY,
                        1.0f,
                        Fade(DARKBLUE, 0.055f)
                    );
                }
            }

            DrawCircleGradient(
                screenWidth / 2,
                70,
                260.0f,
                Fade(SKYBLUE, 0.075f),
                BLANK
            );

            auto MenuEntranceProgress =
                [&](float startTime, float endTime)
            {
                float progress = std::clamp(
                    (menuEntranceTime - startTime) /
                    (endTime - startTime),
                    0.0f,
                    1.0f
                );

                return 1.0f -
                    powf(1.0f - progress, 3.0f);
            };

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

            bool windPressed =
                windHovered &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT);

            bool collisionPressed =
                collisionHovered &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT);

            bool orbitalPressed =
                orbitalHovered &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT);

            bool fluidPressed =
                fluidHovered &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT);

            Color windFill =
                windPressed
                ? Fade(RED, 0.16f)
                : windHovered
                    ? Fade(RED, 0.08f)
                    : WHITE;

            Color collisionFill =
                collisionPressed
                ? Fade(BLUE, 0.16f)
                : collisionHovered
                    ? Fade(BLUE, 0.08f)
                    : WHITE;

            Color orbitalFill =
                orbitalPressed
                ? Fade(GREEN, 0.16f)
                : orbitalHovered
                    ? Fade(GREEN, 0.08f)
                    : WHITE;

            Color fluidFill =
                fluidPressed
                ? Fade(SKYBLUE, 0.18f)
                : fluidHovered
                    ? Fade(SKYBLUE, 0.10f)
                    : WHITE;

            Color windBorder = windHovered
                ? Color{ 255, 65, 85, 255 }
                : RED;

            Color collisionBorder = collisionHovered
                ? Color{ 65, 155, 255, 255 }
                : BLUE;

            Color orbitalBorder = orbitalHovered
                ? Color{ 45, 200, 95, 255 }
                : GREEN;

            Color fluidBorder = fluidHovered
                ? Color{ 85, 205, 255, 255 }
                : SKYBLUE;

            //========================
            // Draw card backgrounds
            //========================

            auto DrawCardShadow =
                [&](Rectangle card,
                    Color accent,
                    bool hovered)
            {
                Rectangle shadow = card;
                shadow.y += hovered ? 4.0f : 2.0f;

                DrawRectangleRounded(
                    shadow,
                    0.04f,
                    12,
                    Fade(accent, hovered ? 0.10f : 0.035f)
                );
            };

            DrawCardShadow(windButton, RED, windHovered);
            DrawCardShadow(collisionButton, BLUE, collisionHovered);
            DrawCardShadow(orbitalButton, GREEN, orbitalHovered);
            DrawCardShadow(fluidButton, SKYBLUE, fluidHovered);

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
                windBorder
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
                collisionBorder
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
                orbitalBorder
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
                fluidBorder
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
                    baseY +
                        sinf(
                            menuAnimationTime * 1.8f +
                            430.0f * 0.025f +
                            baseY * 0.04f
                        ) *
                        0.8f
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
                        baseY +
                            bend +
                            sinf(
                                menuAnimationTime * 1.8f +
                                x * 0.025f +
                                baseY * 0.04f
                            ) *
                            0.8f
                    };

                    float streamlineAlpha =
                        0.42f +
                        horizontalInfluence * 0.18f;

                    DrawLineEx(
                        previousPoint,
                        currentPoint,
                        1.4f,
                        Fade(BLUE, streamlineAlpha)
                    );

                    previousPoint =
                        currentPoint;
                }

                float arrowX = 905.0f;
                float arrowY =
                    baseY +
                    sinf(
                        menuAnimationTime * 1.8f +
                        arrowX * 0.025f +
                        baseY * 0.04f
                    ) *
                    0.8f;

                DrawTriangle(
                    {
                        arrowX,
                        arrowY
                    },
                    {
                        arrowX - 8.0f,
                        arrowY - 4.0f
                    },
                    {
                        arrowX - 8.0f,
                        arrowY + 4.0f
                    },
                    Fade(BLUE, 0.75f)
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

            float collisionMotion =
                (sinf(menuAnimationTime * 2.2f) + 1.0f) *
                0.5f;

            float leftBodyX =
                175.0f + collisionMotion * 6.0f;

            float rightBodyX =
                290.0f - collisionMotion * 6.0f;

            DrawLineEx(
                { leftBodyX - 30.0f, 475.0f },
                { leftBodyX - 10.0f, 475.0f },
                5.0f,
                Fade(BLUE, 0.12f)
            );

            DrawLineEx(
                { rightBodyX + 30.0f, 475.0f },
                { rightBodyX + 10.0f, 475.0f },
                5.0f,
                Fade(RED, 0.12f)
            );

            DrawCircle(leftBodyX, 475, 29, Fade(BLUE, 0.08f));
            DrawCircle(rightBodyX, 475, 29, Fade(RED, 0.08f));

            DrawCircle(
                leftBodyX,
                475,
                22,
                BLUE
            );

            DrawCircle(
                rightBodyX,
                475,
                22,
                RED
            );

            DrawLineEx(
                { leftBodyX + 25.0f, 475.0f },
                { leftBodyX + 46.0f, 475.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { leftBodyX + 46.0f, 475.0f },
                { leftBodyX + 40.0f, 470.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { leftBodyX + 46.0f, 475.0f },
                { leftBodyX + 40.0f, 480.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { rightBodyX - 25.0f, 475.0f },
                { rightBodyX - 46.0f, 475.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { rightBodyX - 46.0f, 475.0f },
                { rightBodyX - 40.0f, 470.0f },
                2.0f,
                BLACK
            );

            DrawLineEx(
                { rightBodyX - 46.0f, 475.0f },
                { rightBodyX - 40.0f, 480.0f },
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

            DrawCircle(525, 475, 38, Fade(YELLOW, 0.07f));
            DrawCircle(525, 475, 31, Fade(ORANGE, 0.11f));

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

            float orbitalAngle =
                menuAnimationTime * 0.45f;

            Vector2 outerPlanetPosition = {
                525.0f + cosf(orbitalAngle) * 85.0f,
                475.0f + sinf(orbitalAngle) * 42.0f
            };

            Vector2 innerPlanetPosition = {
                525.0f + cosf(-orbitalAngle * 1.35f + PI) * 57.0f,
                475.0f + sinf(-orbitalAngle * 1.35f + PI) * 27.0f
            };

            DrawCircleV(
                outerPlanetPosition,
                14,
                Fade(BLUE, 0.09f)
            );

            DrawCircleV(
                outerPlanetPosition,
                9,
                BLUE
            );

            DrawCircleV(
                innerPlanetPosition,
                13,
                Fade(DARKGREEN, 0.10f)
            );

            DrawCircleV(
                innerPlanetPosition,
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

            float fluidCubeRotation =
                -12.0f +
                sinf(menuAnimationTime * 1.4f) * 3.0f;

            Rectangle fluidCubeShadow = fluidCube;
            fluidCubeShadow.x += 3.0f;
            fluidCubeShadow.y += 3.0f;

            DrawRectanglePro(
                fluidCubeShadow,
                {
                    15.0f,
                    15.0f
                },
                fluidCubeRotation,
                Fade(BLACK, 0.12f)
            );

            DrawRectanglePro(
                fluidCube,
                {
                    15.0f,
                    15.0f
                },
                fluidCubeRotation,
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
                    particle.y +
                        45.0f +
                        sinf(
                            menuAnimationTime * 1.8f +
                            particle.x * 0.05f
                        ) *
                        1.5f
                };

                if (CheckCollisionPointRec(
                        menuParticlePosition,
                        fluidButton))
                {
                    float depthAmount = std::clamp(
                        (menuParticlePosition.y - 445.0f) / 80.0f,
                        0.0f,
                        1.0f
                    );

                    Color menuWaterColor = ColorLerp(
                        Color{ 13, 209, 255, 255 },
                        Color{ 0, 51, 148, 255 },
                        depthAmount
                    );

                    DrawCircleV(
                        menuParticlePosition,
                        1.7f,
                        menuWaterColor
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

            float headerProgress =
                MenuEntranceProgress(0.00f, 0.18f);

            float featuredProgress =
                MenuEntranceProgress(0.08f, 0.32f);

            float lowerCardsProgress =
                MenuEntranceProgress(0.18f, 0.44f);

            float footerProgress =
                MenuEntranceProgress(0.30f, 0.50f);

            DrawRectangle(
                0,
                0,
                screenWidth,
                105,
                Fade(menuBackground, 1.0f - headerProgress)
            );

            DrawRectangle(
                0,
                105,
                screenWidth,
                245,
                Fade(menuBackground, 1.0f - featuredProgress)
            );

            DrawRectangle(
                0,
                350,
                screenWidth,
                200,
                Fade(menuBackground, 1.0f - lowerCardsProgress)
            );

            DrawRectangle(
                0,
                550,
                screenWidth,
                50,
                Fade(menuBackground, 1.0f - footerProgress)
            );
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
                menuEntranceTime = 0.0f;
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

            DrawCollisionUIPanels();
            DrawUIOverlay(world, spawnMode, spawnRadius, simulationPaused);
            DrawMassSlider(world);
            DrawSpawnModeButtons(spawnMode);
            DrawSelectedBodyPanel(world, selectedBodyIndex);
            UIText("Backspace: Menu", 24, 585, 16, DARKGRAY);
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
                menuEntranceTime = 0.0f;
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
                menuEntranceTime = 0.0f;
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
                menuEntranceTime = 0.0f;
            }
        }

        EndDrawing();
    }
    UnloadFont(uiFont);
    CloseWindow();

    return 0;
}
