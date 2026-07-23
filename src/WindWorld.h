#pragma once
#include <vector>
#include "raylib.h"

enum class ObstacleShape
{
    Circle,
    Square,
    Diamond,
    Airfoil
};

enum class ParticleVisualization
{
    Speed,
    Pressure
};


struct WindParticle
{
    Vector2 position;
    float radius;
    Vector2 velocity;
    Vector2 spawnPosition;

    std::vector<Vector2> trail;
};


class WindWorld
{

public:
    int screenWidth = 1050;
    int screenHeight = 600;

    void Reset();
    void Update(float dt);
    void Draw();

    Vector2 velocity;

private:
    // Geometry helpers
    Vector2 obstaclePosition;
    float obstacleHalfSize;
    Color obstacleColor;
    float obstacleInfluenceRadius;

    Vector2 RotatePoint(Vector2 point, float angleRadians);

    Vector2 AirfoilLocalToWorld(
        Vector2 localPoint,
        Vector2 airfoilCenter,
        float angleRadians);

    Vector2 AirfoilWorldToLocal(
        Vector2 worldPoint,
        Vector2 airfoilCenter,
        float angleRadians);

    float GetAirFoilThickness(float normalizedX) const;

    // Input
    void HandleInput();

    Rectangle particleCountSlider = {
        20.0f, 100.0f, 130.0f, 8.0f
    };

    Rectangle windSpeedSlider = {
        20.0f, 180.0f, 130.0f, 8.0f
    };

    Rectangle angleSlider = {
        20.0f, 330.0f, 130.0f, 8.0f
    };

    Rectangle circleButton;
    Rectangle squareButton;
    Rectangle diamondButton;
    Rectangle airfoilButton;

    ObstacleShape selectedShape = ObstacleShape::Circle;

    ParticleVisualization particleVisualization =
        ParticleVisualization::Speed;

    Rectangle speedVisualizationButton =
    {
        20.0f,
        245.0f,
        70.0f,
        32.0f
    };

    Rectangle pressureVisualizationButton =
    {
        95.0f,
        245.0f,
        80.0f,
        32.0f
    };

    float minAngle = -20.0f;
    float maxAngle = 20.0f;

    // Particles
    std::vector<WindParticle> particles;

    int particleCount = 500;
    int minParticleCount = 50;
    int maxParticleCount = 1000;

    float windSpeed = 500.0f;
    float minWindSpeed = 100.0f;
    float maxWindSpeed = 1000.0f;

    void SpawnParticle(Vector2 position);
    void UpdateParticles(float dt);

    // Collisions
    void RedirectVelocityAlongSurface(
        WindParticle& particle,
        Vector2 normal);

    void ResolveCircleCollision(WindParticle& particle);
    void ResolveSquareCollision(WindParticle& particle);
    void ResolveDiamondCollision(WindParticle& particle);
    void ResolveAirfoilCollision(WindParticle& particle);
    void ResolveParticleObstacleCollision(WindParticle& particle);
    void ApplyObstacleInfluence(WindParticle& particle, float dt);

    // Aerodynamics
    float airDensity = 1.225f;
    float airfoilSpan = 1.0f;
    float airfoilAngleDegrees = 0.0f;
    float liftCoefficient = 0.0f;
    float liftForce = 0.0f;
    float pixelsPerMeter = 100.0f;
    float liftVectorPixelsPerNewton = 3.0f;
    float dragCoefficient = 0.0f;
    float dragForce = 0.0f;
    float inducedDrag = 0.0f;
    float dragVectorPixelsPerNewton = 30.0f;

    void CalculateLift();
    void CalculateDrag();
    void DrawLiftVector();
    void DrawDragVector();

    // Rendering
    Rectangle tunnel;

    void DrawUI();
    void DrawParticles();
    void DrawCircleObstacle();
    void DrawSquareObstacle();
    void DrawDiamondObstacle();
    void DrawAirfoilObstacle();
};
