//Handles input and rendering
#include "raylib.h"
#include <vector>
#include <cmath>
#include "RigidBody.h"
#include "PhysicsWorld.h"
#include "Physics.h"
#include "OrbitalWorld.h"
#include "FluidWorld.h"
enum class AppState
{
    MainMenu,
    CollisionSim, 
    OrbitalSim, 
    FluidSim
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
    return IsMouseOverMassSlider() ||
           IsMouseOverRect(10, 330, 120, 35) ||
           IsMouseOverRect(140, 330, 120, 35);
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

void DrawDebugOverlay(const PhysicsWorld& world, SpawnMode spawnMode, float spawnRadius)
{
    DrawText(TextFormat("Bodies: %i", world.GetBodyCount()), 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 35, 20, DARKGRAY);

    if (spawnMode == SPAWN_DYNAMIC)
    {
        DrawText("Mode: Dynamic", 10, 60, 20, DARKGRAY);
    }
    else
    {
        DrawText("Mode: Static", 10, 60, 20, DARKGRAY);
    }

    DrawText(TextFormat("Radius: %.0f", spawnRadius), 10, 85, 20, DARKGRAY);

    
    DrawText("[: Smaller   ]: Larger", 10, 145, 20, DARKGRAY);
    DrawText("Left Click: Spawn", 10, 170, 20, DARKGRAY);
    DrawText("Space: Jump   R: Reset", 10, 195, 20, DARKGRAY);
}

void DrawSpawnModeButtons(SpawnMode& spawnMode)
{
    float x = 10;
    float y = 330;
    float width = 120;
    float height = 35;

    bool mouseOverDynamic = IsMouseOverRect(x, y, width, height);
    bool mouseOverStatic = IsMouseOverRect(x + width + 10, y, width, height);

    Color dynamicColor = LIGHTGRAY;
    Color staticColor = LIGHTGRAY;

    if (spawnMode == SPAWN_DYNAMIC)
    {
        dynamicColor = SKYBLUE;
    }

    if (spawnMode == SPAWN_STATIC)
    {
        staticColor = DARKGRAY;
    }

    DrawRectangle(x, y, width, height, dynamicColor);
    DrawRectangle(x + width + 10, y, width, height, staticColor);

    DrawRectangleLines(x, y, width, height, DARKGRAY);
    DrawRectangleLines(x + width + 10, y, width, height, DARKGRAY);

    DrawText("Dynamic", x + 15, y + 8, 20, BLACK);
    DrawText("Static", x + width + 35, y + 8, 20, BLACK);

    if (mouseOverDynamic && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        spawnMode = SPAWN_DYNAMIC;
    }

    if (mouseOverStatic && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        spawnMode = SPAWN_STATIC;
    }
}

void DrawMassSlider(PhysicsWorld& world)
{
    if (world.GetBodyCount() == 0)
    {
        return;
    }

    RigidBody& player = world.GetBody(0);

    float sliderX = 10;
    float sliderY = 275;
    float sliderWidth = 200;
    float sliderHeight = 8;

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

    DrawText(TextFormat("Player Mass: %.2f", player.mass), 10, sliderY - 30, 20, DARKGRAY);

    DrawRectangle(sliderX, sliderY, sliderWidth, sliderHeight, LIGHTGRAY);
    DrawCircle(knobX, sliderY + sliderHeight / 2, 8, DARKGRAY);
}

bool IsMouseOverMassSlider()
{
    float sliderX = 10;
    float sliderY = 275;
    float sliderWidth = 200;
    float sliderHeight = 8;

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

    int screenWidth = 800;
    int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Physics Engine");
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

    const float fixedDt = 1.0f / 60.0f;
    float accumulator = 0.0f;

    SpawnMode spawnMode = SPAWN_DYNAMIC;
    float spawnRadius = 20.0f;

    while (!WindowShouldClose())
    {
        float frameTime = GetFrameTime();

        BeginDrawing();

        if (currentState == AppState::MainMenu)
    {
        ClearBackground({245, 247, 250, 255});

        DrawText("PHYSICS SANDBOX", 190, 55, 44, BLACK);
        DrawText("Choose a simulation to begin", 255, 110, 20, GRAY);

        Rectangle collisionButton = { 40, 170, 220, 320 };
        Rectangle orbitalButton   = { 290, 170, 220, 320 };
        Rectangle fluidButton     = { 540, 170, 220, 320 };

        Vector2 mouse = GetMousePosition();

        Color collisionFill =
            CheckCollisionPointRec(mouse, collisionButton)
            ? Fade(BLUE, 0.08f)
            : WHITE;

        Color orbitalFill =
            CheckCollisionPointRec(mouse, orbitalButton)
            ? Fade(GREEN, 0.08f)
            : WHITE;

        Color fluidFill =
            CheckCollisionPointRec(mouse, fluidButton)
            ? Fade(SKYBLUE, 0.08f)
            : WHITE;

        DrawRectangleRounded(collisionButton, 0.04f, 12, collisionFill);
        DrawRectangleRounded(orbitalButton, 0.04f, 12, orbitalFill);
        DrawRectangleRounded(fluidButton, 0.04f, 12, fluidFill);

        DrawRectangleRoundedLines(collisionButton, 0.04f, 12, BLUE);
        DrawRectangleRoundedLines(orbitalButton, 0.04f, 12, GREEN);
        DrawRectangleRoundedLines(fluidButton, 0.04f, 12, SKYBLUE);

        //========================
        // Collision Card
        //========================

        DrawText("RIGID BODY", 65, 205, 24, BLUE);
        DrawText("COLLISION SIM", 65, 235, 24, BLUE);

        DrawCircle(105, 350, 28, BLUE);
        DrawCircle(195, 350, 28, RED);

        // Blue ->
        DrawLineEx({135, 350}, {148, 350}, 2.0f, BLACK);
        DrawLineEx({148, 350}, {144, 347}, 2.0f, BLACK);
        DrawLineEx({148, 350}, {144, 353}, 2.0f, BLACK);

        // <- Red
        DrawLineEx({165, 350}, {152, 350}, 2.0f, BLACK);
        DrawLineEx({152, 350}, {156, 347}, 2.0f, BLACK);
        DrawLineEx({152, 350}, {156, 353}, 2.0f, BLACK);

        DrawCircle(150, 435, 22, GREEN);

        DrawLine(90, 460, 210, 460, BLACK);

        //========================
        // Orbital Card
        //========================

        DrawText("ORBITAL SIM", 315, 220, 24, GREEN);

        DrawCircle(400, 365, 30, YELLOW);

        DrawEllipseLines(400, 365, 95, 55, BLUE);
        DrawEllipseLines(400, 365, 65, 35, DARKGREEN);

        DrawCircle(475, 310, 12, BLUE);
        DrawCircle(330, 425, 10, DARKGREEN);

        //========================
        // Fluid Card
        //========================

        DrawText("FLUID SIM", 565, 220, 24, SKYBLUE);

        Rectangle cube = { 620, 400, 35, 35 };

        DrawRectanglePro(
            cube,
            { 17.5f, 17.5f },   // center of rotation
            -12.0f,             // rotation angle
            ORANGE
        );
        DrawCircle(705, 400, 18, LIGHTGRAY);

        for (const Vector2& particle : menuFluidParticles)
        {
            DrawCircleV(particle, 2, BLUE);
        }

        DrawText("Use mouse to select", 285, 540, 22, GRAY);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mouse, collisionButton))
            {
                currentState = AppState::CollisionSim;
            }

            if (CheckCollisionPointRec(mouse, orbitalButton))
            {
                currentState = AppState::OrbitalSim;
            }

            if (CheckCollisionPointRec(mouse, fluidButton))
            {
                currentState = AppState::FluidSim;
            }
        }
    }

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

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsMouseOverUI())
            {
                SpawnBodyAtMouse(world, spawnMode, spawnRadius);
            }

            if (IsKeyPressed(KEY_R))
            {
                world.Reset();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                currentState = AppState::MainMenu;
            }

            while (accumulator >= fixedDt)
            {
                world.Step(fixedDt);
                accumulator -= fixedDt;
            }

            world.Draw();
            DrawDebugOverlay(world, spawnMode, spawnRadius);
            DrawMassSlider(world);
            DrawSpawnModeButtons(spawnMode);
            DrawText("BACKSPACE: Menu", 20, 560, 20, GRAY);
        }

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

        EndDrawing();
    }

    CloseWindow();

    return 0;
}