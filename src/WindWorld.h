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
    std::vector<WindParticle> particles;
    void SpawnParticle(Vector2 position);
    void UpdateParticles(float dt);
    void DrawUI();

    Rectangle tunnel;
    Rectangle particleCountSlider = {
        20.0f, 100.0f, 130.0f, 8.0f
    };
    Rectangle windSpeedSlider = {
        20.0f, 180.0f, 130.0f, 8.0f
    };

    Vector2 obstaclePosition;
    float   obstacleHalfSize;
    Color obstacleColor;
    float obstacleInfluenceRadius;

    void ResolveParticleObstacleCollision (WindParticle& particle);
    void ApplyObstacleInfluence(WindParticle& particle, float dt);
    void HandleInput();
    

    int particleCount = 500;
    int minParticleCount = 50;
    int maxParticleCount = 1000;

    float windSpeed = 500.0f;
    float minWindSpeed = 100.0f;
    float maxWindSpeed = 1000.0f;

    Rectangle circleButton;
    Rectangle squareButton;
    Rectangle diamondButton;
    Rectangle airfoilButton;
    ObstacleShape selectedShape = ObstacleShape::Circle;

    
    
};
