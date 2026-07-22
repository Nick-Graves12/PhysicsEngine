#include "WindWorld.h"
#include "raylib.h"
#include "UI.h"
#include <algorithm>
#include "raymath.h"

void WindWorld::Reset()
{
    tunnel.x = 180;
    tunnel.y = 80;
    tunnel.width = 800;
    tunnel.height = 440;

    velocity= { windSpeed, 0};

    obstaclePosition = {570.0f, 300.0f};
    obstacleHalfSize = 40.0f;
    obstacleInfluenceRadius = 120.0f;
    obstacleColor = RED;

    circleButton  = { 220, 20, 90, 32 };
    squareButton  = { 320, 20, 90, 32 };
    diamondButton = { 420, 20, 90, 32 };
    airfoilButton = { 520, 20, 90, 32 };


    particles.clear();

    for (int particleIndex = 0; particleIndex < particleCount; particleIndex++)
    {
        Vector2 spawnPosition = {
            static_cast<float>(GetRandomValue(
                static_cast<int>(tunnel.x + 10.0f),
                static_cast<int>(tunnel.x + tunnel.width - 10.0f)
            )),
            static_cast<float>(GetRandomValue(
                static_cast<int>(tunnel.y + 10.0f),
                static_cast<int>(tunnel.y + tunnel.height - 10.0f)
            ))
        };

        SpawnParticle(spawnPosition);
    }
}

void WindWorld::Update(float dt)
{
    HandleInput();
    CalculateLift();
    CalculateDrag();
    UpdateParticles(dt);
}


//Geometry Helpers
//------------
Vector2 WindWorld::RotatePoint(Vector2 point, float angleRadians)
{
    float cosine = cosf(angleRadians);
    float sine = sinf(angleRadians);

    return
    {
        point.x * cosine - point.y * sine,
        point.x * sine + point.y * cosine
    };
}

Vector2 WindWorld::AirfoilLocalToWorld(Vector2 localPoint, Vector2 airfoilCenter, float angleRadians)
{
    Vector2 rotatedPoint =
        RotatePoint(localPoint, angleRadians);

    return
    {
        airfoilCenter.x + rotatedPoint.x,
        airfoilCenter.y + rotatedPoint.y
    };
}

Vector2 WindWorld::AirfoilWorldToLocal(Vector2 worldPoint, Vector2 airfoilCenter, float angleRadians)
{
    Vector2 relativePoint =
    {
        worldPoint.x - airfoilCenter.x,
        worldPoint.y - airfoilCenter.y
    };

    return RotatePoint(relativePoint, -angleRadians);
}

//Input
//------------
void WindWorld::HandleInput()
{
    Vector2 mousePosition = GetMousePosition();

    Rectangle sliderHitbox = {
        particleCountSlider.x,
        particleCountSlider.y - 8.0f,
        particleCountSlider.width,
        particleCountSlider.height + 16.0f
    };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePosition, sliderHitbox))
    {
        float mouseAmount =
            (mousePosition.x - particleCountSlider.x) /
            particleCountSlider.width;

        mouseAmount = std::clamp(mouseAmount, 0.0f, 1.0f);

        particleCount =
            minParticleCount +
            static_cast<int>(
                mouseAmount *
                (maxParticleCount - minParticleCount)
            );
    }

    Rectangle windSliderHitbox = {
        windSpeedSlider.x,
        windSpeedSlider.y - 8.0f,
        windSpeedSlider.width,
        windSpeedSlider.height + 16.0f
        };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePosition, windSliderHitbox))
    {
        float mouseAmount =
            (mousePosition.x - windSpeedSlider.x) /
            windSpeedSlider.width;

        mouseAmount = std::clamp(mouseAmount, 0.0f, 1.0f);

        windSpeed =
            minWindSpeed +
            mouseAmount * (maxWindSpeed - minWindSpeed);

        velocity.x = windSpeed;
        velocity.y = 0.0f;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePosition = GetMousePosition();

        if (CheckCollisionPointRec(
            mousePosition,
            speedVisualizationButton))
        {
            particleVisualization =
                ParticleVisualization::Speed;
        }

        if (CheckCollisionPointRec(
            mousePosition,
            pressureVisualizationButton))
        {
            particleVisualization =
                ParticleVisualization::Pressure;
        }
    }


    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mousePosition, circleButton))
        {
            selectedShape = ObstacleShape::Circle;
            Reset();
        }

        if (CheckCollisionPointRec(mousePosition, squareButton))
        {
            selectedShape = ObstacleShape::Square;
            Reset();
        }

        if (CheckCollisionPointRec(mousePosition, diamondButton))
        {
            selectedShape = ObstacleShape::Diamond;
            Reset();
        }

        if (CheckCollisionPointRec(mousePosition, airfoilButton))
        {
            selectedShape = ObstacleShape::Airfoil;
            Reset();
        }
    }

    if (selectedShape == ObstacleShape::Airfoil)
    {
        Rectangle angleOfAttackHitbox = {
        angleSlider.x,
        angleSlider.y - 8.0f,
        angleSlider.width,
        angleSlider.height + 16.0f
        };

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(mousePosition, angleOfAttackHitbox))
        {


            float mouseAmount =
                (mousePosition.x - angleSlider.x) /
                angleSlider.width;

            mouseAmount = std::clamp(mouseAmount, 0.0f, 1.0f);

            airfoilAngleDegrees =
                minAngle +
                mouseAmount *
                (maxAngle - minAngle);
        }
    }  
}


//Particles
//------------
void WindWorld::SpawnParticle(Vector2 spawnPosition)
{
    WindParticle particle;

    particle.position = spawnPosition;
    particle.spawnPosition = particle.position;
    particle.velocity = velocity;
    particle.radius = 2.0f;

    particles.push_back(particle);
}

void WindWorld::UpdateParticles(float dt)
{
    float recoveryStrength = 6.0f;

    for (WindParticle& particle : particles)
    {
        ApplyObstacleInfluence(particle, dt);

        particle.position.x +=  particle.velocity.x * dt;
        particle.position.y +=  particle.velocity.y * dt;

        ResolveParticleObstacleCollision(particle);

        
        float dx = particle.position.x - obstaclePosition.x;
        float dy = particle.position.y - obstaclePosition.y;
        float distanceToObstacle = sqrt(dx * dx + dy * dy);

        bool insideInfluence = 
            distanceToObstacle <= obstacleInfluenceRadius;

        if (!insideInfluence)
        {
            Vector2 difference = {velocity.x - particle.velocity.x, velocity.y - particle.velocity.y};

            particle.velocity.x += difference.x * recoveryStrength * dt;
            particle.velocity.y += difference.y * recoveryStrength * dt;
        }

        particle.trail.push_back(particle.position);

        if (particle.trail.size() > 50)
        {
            particle.trail.erase(particle.trail.begin());
        }

        if (particle.position.x > tunnel.x + tunnel.width)
        {   
            particle.position.x = tunnel.x + 10.0f;
            particle.position.y = GetRandomValue(
                static_cast<int>(tunnel.y + 10.0f),
                static_cast<int>(tunnel.y + tunnel.height - 10.0f)
            );

            particle.velocity = velocity;
            particle.trail.clear();
        }
    }
}


//Collisions
//------------
void WindWorld::RedirectVelocityAlongSurface(WindParticle& particle, Vector2 normal)
{
        Vector2 tangentA = {-normal.y, normal.x};
        Vector2 tangentB = {normal.y, -normal.x};
        float dotA = tangentA.x * particle.velocity.x + tangentA.y * particle.velocity.y;
        float dotB = tangentB.x * particle.velocity.x + tangentB.y * particle.velocity.y;
        Vector2 tangent = (dotA > dotB) ? tangentA : tangentB;

        float speed = sqrt(particle.velocity.x * particle.velocity.x + particle.velocity.y * particle.velocity.y);
                
        particle.velocity.x = tangent.x * speed;
        particle.velocity.y = tangent.y * speed;
}

void WindWorld::ResolveCircleCollision(WindParticle& particle)
{
    float dx = particle.position.x - obstaclePosition.x;
    float dy = particle.position.y - obstaclePosition.y;
    float distance = sqrt(dx * dx + dy * dy);
            
    float radiusSum = particle.radius + obstacleHalfSize;
            

    if (distance <= radiusSum && distance > 0.0f)
    {
        float penetration = radiusSum - distance;
        Vector2 normal = 
        {
            dx / distance,
            dy / distance
        };

        particle.position.x += normal.x * penetration;
        particle.position.y += normal.y * penetration;

        RedirectVelocityAlongSurface(particle, normal);
    }
}

void WindWorld::ResolveSquareCollision(WindParticle& particle)
{
    float leftEdge = obstaclePosition.x - obstacleHalfSize;
    float rightEdge = obstaclePosition.x + obstacleHalfSize;
    float top = obstaclePosition.y - obstacleHalfSize;
    float bottom = obstaclePosition.y + obstacleHalfSize;

    if (particle.position.x - particle.radius >= leftEdge && 
        particle.position.x + particle.radius<= rightEdge && 
        particle.position.y - particle.radius >= top && 
        particle.position.y + particle.radius <= bottom)
            {
                float distanceFromLeftEdge = (particle.position.x - particle.radius) - leftEdge;
                float distanceFromRightEdge = rightEdge - (particle.position.x + particle.radius);
                float distanceFromTop = (particle.position.y - particle.radius) - top;
                float distanceFromBottom = bottom - (particle.position.y + particle.radius);

                float smallest = std::min({distanceFromLeftEdge, 
                    distanceFromRightEdge, 
                    distanceFromTop, 
                    distanceFromBottom});

                Vector2 normal = {0.0f, 0.0f};

                if (smallest == distanceFromLeftEdge)
                {
                    particle.position.x = leftEdge + particle.radius;
                    normal = {-1.0f, 0.0f};
                }
                else if (smallest == distanceFromRightEdge)
                {
                    particle.position.x = rightEdge - particle.radius;
                    normal = {1.0f, 0.0f};
                }
                else if (smallest == distanceFromTop)
                {
                    particle.position.y = top + particle.radius;
                   normal = {0.0f, -1.0f};
                }
                else if (smallest == distanceFromBottom)
                {
                    particle.position.y = bottom - particle.radius;
                    normal = {0.0f, 1.0f};
                }

                RedirectVelocityAlongSurface(particle, normal);
            }

}

void WindWorld::ResolveDiamondCollision(WindParticle& particle)
{
    float dx = particle.position.x - obstaclePosition.x;
    float dy = particle.position.y - obstaclePosition.y;

    float diamondRadius = obstacleHalfSize * 1.41421356f;
    const float inverseSqrtTwo = 0.70710678f;
    const float sqrtTwo = 1.41421356f;

    // Expands the diamond by the particle's radius.
    float expandedRadius =
        diamondRadius + particle.radius * sqrtTwo;

    float diamondDistance = fabs(dx) + fabs(dy);

    if (diamondDistance <= expandedRadius)
        {
                Vector2 normal = { 0.0f, 0.0f };

                // Choose the face based on the particle's quadrant.
                if (dx <= 0.0f && dy <= 0.0f)
                {
                    // Upper-left face
                    normal = { -inverseSqrtTwo, -inverseSqrtTwo };
                }
                else if (dx >= 0.0f && dy <= 0.0f)
                {
                    // Upper-right face
                    normal = { inverseSqrtTwo, -inverseSqrtTwo };
                }
                else if (dx <= 0.0f && dy >= 0.0f)
                {
                    // Lower-left face
                    normal = { -inverseSqrtTwo, inverseSqrtTwo };
                }
                else
                {
                    // Lower-right face
                    normal = { inverseSqrtTwo, inverseSqrtTwo };
                }

                float penetration =
                    (expandedRadius - diamondDistance) * inverseSqrtTwo;

                particle.position.x += normal.x * penetration;
                particle.position.y += normal.y * penetration;

                RedirectVelocityAlongSurface(particle, normal);
        }
}

void WindWorld::ResolveAirfoilCollision(WindParticle& particle)
{
            const int segmentCount = 32;

            const float chord = obstacleHalfSize * 3.0f;
            const float thicknessRatio = 0.12f;
            float airfoilAngleRadians =
                airfoilAngleDegrees * DEG2RAD;

            float closestDistanceSquared = 1000000000.0f;

            Vector2 closestPoint = particle.position;
            Vector2 closestTangent = { 1.0f, 0.0f };
            Vector2 closestNormal = { 0.0f, -1.0f };

            // Finds the closest point on one surface segment.
            auto CheckSurfaceSegment =
                [&](Vector2 segmentStart,
                    Vector2 segmentEnd,
                    bool upperSurface)
            {
                Vector2 segment =
                {
                    segmentEnd.x - segmentStart.x,
                    segmentEnd.y - segmentStart.y
                };

                Vector2 pointOffset =
                {
                    particle.position.x - segmentStart.x,
                    particle.position.y - segmentStart.y
                };

                float segmentLengthSquared =
                    segment.x * segment.x +
                    segment.y * segment.y;

                if (segmentLengthSquared <= 0.0f)
                {
                    return;
                }

                float segmentAmount =
                    (
                        pointOffset.x * segment.x +
                        pointOffset.y * segment.y
                    ) /
                    segmentLengthSquared;

                segmentAmount =
                    std::clamp(segmentAmount, 0.0f, 1.0f);

                Vector2 pointOnSegment =
                {
                    segmentStart.x + segment.x * segmentAmount,
                    segmentStart.y + segment.y * segmentAmount
                };

                float dx =
                    particle.position.x - pointOnSegment.x;

                float dy =
                    particle.position.y - pointOnSegment.y;

                float distanceSquared = dx * dx + dy * dy;

                if (distanceSquared < closestDistanceSquared)
                {
                    closestDistanceSquared = distanceSquared;
                    closestPoint = pointOnSegment;

                    float segmentLength =
                        sqrt(
                            segment.x * segment.x +
                            segment.y * segment.y
                        );

                    closestTangent =
                    {
                        segment.x / segmentLength,
                        segment.y / segmentLength
                    };

                    if (upperSurface)
                    {
                        closestNormal =
                        {
                            closestTangent.y,
                            -closestTangent.x
                        };
                    }
                    else
                    {
                        closestNormal =
                        {
                            -closestTangent.y,
                            closestTangent.x
                        };
                    }
                }
            };

            // Check every upper and lower airfoil segment.
            for (int i = 0; i < segmentCount; i++)
            {
                float x1 =
                    static_cast<float>(i) /
                    static_cast<float>(segmentCount);

                float x2 =
                    static_cast<float>(i + 1) /
                    static_cast<float>(segmentCount);

                float thickness1 =
                    5.0f * thicknessRatio * chord *
                    (
                        0.2969f * sqrtf(x1) -
                        0.1260f * x1 -
                        0.3516f * x1 * x1 +
                        0.2843f * x1 * x1 * x1 -
                        0.1036f * x1 * x1 * x1 * x1
                    );

                float thickness2 =
                    5.0f * thicknessRatio * chord *
                    (
                        0.2969f * sqrtf(x2) -
                        0.1260f * x2 -
                        0.3516f * x2 * x2 +
                        0.2843f * x2 * x2 * x2 -
                        0.1036f * x2 * x2 * x2 * x2
                    );

                Vector2 upperStartLocal =
                {
                    -chord * 0.5f + x1 * chord,
                    -thickness1
                };

                Vector2 upperEndLocal =
                {
                    -chord * 0.5f + x2 * chord,
                    -thickness2
                };

                Vector2 lowerStartLocal =
                {
                    -chord * 0.5f + x1 * chord,
                    thickness1
                };

                Vector2 lowerEndLocal =
                {
                    -chord * 0.5f + x2 * chord,
                    thickness2
                };

                Vector2 upperStart =
                    AirfoilLocalToWorld(
                        upperStartLocal,
                        obstaclePosition,
                        airfoilAngleRadians
                    );

                Vector2 upperEnd =
                    AirfoilLocalToWorld(
                        upperEndLocal,
                        obstaclePosition,
                        airfoilAngleRadians
                    );

                Vector2 lowerStart =
                    AirfoilLocalToWorld(
                        lowerStartLocal,
                        obstaclePosition,
                        airfoilAngleRadians
                    );

                Vector2 lowerEnd =
                    AirfoilLocalToWorld(
                        lowerEndLocal,
                        obstaclePosition,
                        airfoilAngleRadians
                    );

                Vector2 localParticlePosition =
                    AirfoilWorldToLocal(
                        particle.position,
                        obstaclePosition,
                        airfoilAngleRadians
                    );

                bool useUpperSurface =
                    localParticlePosition.y < 0.0f;

                if (useUpperSurface)
                {
                    CheckSurfaceSegment(
                        upperStart,
                        upperEnd,
                        true
                    );
                }
                else
                {
                    CheckSurfaceSegment(
                        lowerStart,
                        lowerEnd,
                        false
                    );
                }
            }

            // Determine whether the particle center is inside the airfoil.
            Vector2 localParticlePosition =
                AirfoilWorldToLocal(
                    particle.position,
                    obstaclePosition,
                    airfoilAngleRadians
                );

            float normalizedX =
                (localParticlePosition.x + chord * 0.5f) / chord;

            bool insideAirfoil = false;

            if (normalizedX >= 0.0f && normalizedX <= 1.0f)
            {
                float x = normalizedX;

                float thickness =
                    5.0f * thicknessRatio * chord *
                    (
                        0.2969f * sqrtf(x) -
                        0.1260f * x -
                        0.3516f * x * x +
                        0.2843f * x * x * x -
                        0.1036f * x * x * x * x
                    );

                float localY =
                    localParticlePosition.y;

                insideAirfoil = fabs(localY) <= thickness;
            }

            float particleRadiusSquared =
                particle.radius * particle.radius;

            if (insideAirfoil ||
                closestDistanceSquared <= particleRadiusSquared)
            {
                // Move the particle just outside the surface.
                particle.position.x =
                    closestPoint.x +
                    closestNormal.x * particle.radius;

                particle.position.y =
                    closestPoint.y +
                    closestNormal.y * particle.radius;

                // Measure how much velocity points into the airfoil.
                float velocityIntoSurface =
                    particle.velocity.x * closestNormal.x +
                    particle.velocity.y * closestNormal.y;

                // Remove only the inward component.
                if (velocityIntoSurface < 0.0f)
                {
                    particle.velocity.x -=
                        closestNormal.x * velocityIntoSurface;

                    particle.velocity.y -=
                        closestNormal.y * velocityIntoSurface;
                }

                // Choose the tangent direction that points generally downstream.
                Vector2 surfaceTangent = closestTangent;

                float tangentWithWind =
                    surfaceTangent.x * velocity.x +
                    surfaceTangent.y * velocity.y;

                if (tangentWithWind < 0.0f)
                {
                    surfaceTangent.x *= -1.0f;
                    surfaceTangent.y *= -1.0f;
                }

                float currentTangentialSpeed =
                    particle.velocity.x * surfaceTangent.x +
                    particle.velocity.y * surfaceTangent.y;

                float minimumSurfaceSpeed = windSpeed * 0.5f;

                if (currentTangentialSpeed < minimumSurfaceSpeed)
                {
                    float neededSpeed =
                        minimumSurfaceSpeed - currentTangentialSpeed;

                    particle.velocity.x +=
                        surfaceTangent.x * neededSpeed;

                    particle.velocity.y +=
                        surfaceTangent.y * neededSpeed;
                }
            }

}

void WindWorld::ResolveParticleObstacleCollision(WindParticle& particle)
{   
    switch (selectedShape)
    {
        case ObstacleShape::Circle:
        {
            ResolveCircleCollision(particle);
            break;
        }

        case ObstacleShape::Square:
        {
            ResolveSquareCollision(particle);
            break;
        }

        case ObstacleShape::Diamond:
        {
            ResolveDiamondCollision(particle);
            break;
        }

        case ObstacleShape::Airfoil:
        {
            ResolveAirfoilCollision(particle);
            break;
        }
    }


}

void WindWorld::ApplyObstacleInfluence(WindParticle& particle, float dt)
{
    float dx = particle.position.x - obstaclePosition.x;
    float dy = particle.position.y - obstaclePosition.y;
    float distance = sqrt(dx * dx + dy * dy);

    float normalY = dy / distance;
    float sideAmount = fabs(normalY);
    float speed = sqrt(
            particle.velocity.x * particle.velocity.x +
            particle.velocity.y * particle.velocity.y
        );

    float slowdownStrength = windSpeed * 0.20f;
    float deflectionStrength = windSpeed * 0.14f;
    float sideAccelerationStrength = 0.6f;
    float wakeExpansion = 0.35f;
    float wakeLength = 320.0f;

    if (distance <= obstacleInfluenceRadius)
    {
        float influenceAmount = 
            (obstacleInfluenceRadius - distance) / (obstacleInfluenceRadius - obstacleHalfSize);
    
        influenceAmount = std::clamp(influenceAmount, 0.0f, 1.0f);
        influenceAmount = sqrt(influenceAmount);
        float accelerationAmount = influenceAmount * sideAmount;

        if (particle.position.x < obstaclePosition.x)
        {
            particle.velocity.x -= slowdownStrength * influenceAmount * dt;
            particle.velocity.x = std::clamp(particle.velocity.x, windSpeed * 0.35f, windSpeed);
        }
        if (particle.position.x < obstaclePosition.x)
        {
            if (dy > 0.0f)
            {
                particle.velocity.y += deflectionStrength * influenceAmount * dt;
            }
            else if (dy < 0.0f)
            {
                particle.velocity.y -= deflectionStrength * influenceAmount * dt;
            }
        }
        if (particle.position.x < obstaclePosition.x)
        {
            if (speed > 0.0f)
            {
                float currentParticleSpeed = sqrt(
                    particle.velocity.x * particle.velocity.x +
                    particle.velocity.y * particle.velocity.y
                );

                float directionX =
                    particle.velocity.x / currentParticleSpeed;

                float directionY =
                    particle.velocity.y / currentParticleSpeed;

                float actualSideAcceleration =
                    windSpeed * sideAccelerationStrength;

                particle.velocity.x +=
                    directionX *
                    actualSideAcceleration *
                    accelerationAmount *
                    dt;

                particle.velocity.y +=
                    directionY *
                    actualSideAcceleration *
                    accelerationAmount *
                    dt;

                float currentSpeed = sqrt(
                    particle.velocity.x * particle.velocity.x +
                    particle.velocity.y * particle.velocity.y
                );

                float maxSpeed = windSpeed * 1.4f;

                if (currentSpeed > maxSpeed)
                {
                    float scale = maxSpeed / currentSpeed;

                    particle.velocity.x *= scale;
                    particle.velocity.y *= scale;
                }
            }
        }
    }

    float distanceBehind = particle.position.x - (obstaclePosition.x + obstacleHalfSize);

    float wakeHalfHeight = obstacleHalfSize + distanceBehind * wakeExpansion;

    float frequency = 5.0f;
    float disturbanceStrength = windSpeed * 0.06f;
    float recoveryStrength = 25.0f;

    if (distanceBehind > 0.0f && distanceBehind < wakeLength)
    {
        float downStreamAmount =
            distanceBehind / wakeLength;

        downStreamAmount =
            std::clamp(
                downStreamAmount,
                0.0f,
                1.0f
            );

        // Wake slowdown only applies inside the wake cone.
        if (fabs(dy) < wakeHalfHeight)
        {
            float distanceFromCenter =
                fabs(dy);

            float centerAmount =
                1.0f -
                (distanceFromCenter / wakeHalfHeight);

            centerAmount = std::clamp(centerAmount, 0.0f, 1.0f);

            float slowdownCenterAmount =
                centerAmount * centerAmount;

            float wakeAmount =
                1.0f - downStreamAmount;

            float wakeMultiplier =
                1.0f -
                (0.10f *
                wakeAmount *
                slowdownCenterAmount);

            particle.velocity.x *= wakeMultiplier;

            
            float verticalDisturbance =
                sin(GetTime() * frequency) *
                disturbanceStrength *
                centerAmount *
                wakeAmount;

            particle.velocity.y +=
                verticalDisturbance * dt;
            
        }

        // Recovery applies to the wider downstream region.
        float normalizedDistance =
            std::clamp(
                fabs(dy) / obstacleInfluenceRadius,
                0.0f,
                1.0f
            );

        float recoveryStrength = 4.0f;
        float recoveryDamping = 2.0f;

        float recoveryAcceleration =
            (-dy * recoveryStrength) -
            (particle.velocity.y * recoveryDamping);

        

        // Alternating turbulence traveling downstream.
        float turbulenceStrength = windSpeed * 0.36f;
        float turbulenceFrequency = 5.0f;
        float turbulenceWaveLength = 70.0f;

        float phase =
            static_cast<float>(GetTime()) * turbulenceFrequency -
            distanceBehind / turbulenceWaveLength;
        
        float sideSign = (dy >= 0.0f) ? 1.0f : -1.0f;

        // Weak at both ends of the wake and strongest near the middle.
        float turbulenceEnvelope =
            sinf(downStreamAmount * PI);

        float turbulenceAcceleration =
            sinf(phase) *
            turbulenceStrength *
            turbulenceEnvelope *
            sideSign;

        particle.velocity.y +=
            recoveryAcceleration *
            downStreamAmount *
            dt;

        particle.velocity.y +=
            turbulenceAcceleration * dt;
    }
}


//Aerodynamics
//------------
void WindWorld::CalculateLift()
{
    float angleRadians =
        airfoilAngleDegrees * DEG2RAD;

    float absoluteAngle =
    fabsf(airfoilAngleDegrees);

    float stallAngle = 15.0f;

    if (absoluteAngle <= stallAngle)
    {
        liftCoefficient =
            2.0f * PI * angleRadians;
    }
    else
    {
        float stallAmount =
            std::clamp(
                (absoluteAngle - stallAngle) / (maxAngle - stallAngle),
                0.0f,
                1.0f
            );

        float maximumLiftCoefficient = 1.4f;

        float stalledLiftCoefficient =
            maximumLiftCoefficient *
            (1.0f - stallAmount * 0.65f);

        liftCoefficient =
            airfoilAngleDegrees >= 0.0f
            ? stalledLiftCoefficient
            : -stalledLiftCoefficient;
    }

    liftCoefficient =
    std::clamp(
        liftCoefficient,
        -1.4f,
        1.4f
    );

    float chordMeters =
        (obstacleHalfSize * 3.0f) /
        pixelsPerMeter;

    float referenceArea =
        chordMeters * airfoilSpan;

    float windSpeedMetersPerSecond =
        windSpeed / pixelsPerMeter;

    float dynamicPressure =
        0.5f *
        airDensity *
        windSpeedMetersPerSecond *
        windSpeedMetersPerSecond;

    liftForce =
        dynamicPressure *
        referenceArea *
        liftCoefficient;
}

void WindWorld::CalculateDrag()
{
    float baseDrag = 0.02f;
    float inducedDragFactor = 0.08f;


    float absoluteAngle =
        fabsf(airfoilAngleDegrees);

    float stallDrag = 0.0f;

    float stallAngle = 15.0f;

    if (absoluteAngle > stallAngle)
    {
        float stallAmount =
            std::clamp(
                (absoluteAngle - stallAngle) / (maxAngle - stallAngle),
                0.0f,
                1.0f
            );

        stallDrag =
            stallAmount * 0.8f;
    }

    float chordMeters =
        (obstacleHalfSize * 3.0f) /
        pixelsPerMeter;

    float referenceArea =
        chordMeters * airfoilSpan;

    float windSpeedMetersPerSecond =
        windSpeed / pixelsPerMeter;

    float dynamicPressure =
        0.5f *
        airDensity *
        windSpeedMetersPerSecond *
        windSpeedMetersPerSecond;

    inducedDrag = liftCoefficient * liftCoefficient * inducedDragFactor;

    dragCoefficient = baseDrag + inducedDrag + stallDrag;

    dragForce =
        dynamicPressure *
        referenceArea *
        dragCoefficient;
}

void WindWorld::DrawLiftVector()
{
    if (selectedShape != ObstacleShape::Airfoil)
    {
        return;
    }

    // Convert lift force into a visible arrow length.
    float signedArrowLength =
        liftForce * liftVectorPixelsPerNewton;

    signedArrowLength =
        std::clamp(
            signedArrowLength,
            -110.0f,
            110.0f
        );

    // Do not draw an arrowhead when lift is nearly zero.
    if (fabsf(signedArrowLength) < 1.0f)
    {
        DrawCircleV(
            obstaclePosition,
            4.0f,
            BLACK
        );

        return;
    }

    Vector2 arrowStart =
    {
        obstaclePosition.x,
        obstaclePosition.y
    };

    Vector2 arrowEnd =
    {
        obstaclePosition.x,

        // Positive lift points upward on the screen.
        obstaclePosition.y - signedArrowLength
    };

    DrawLineEx(
        arrowStart,
        arrowEnd,
        4.0f,
        BLACK
    );

    // Find the arrow's direction.
    Vector2 arrowDirection =
    {
        arrowEnd.x - arrowStart.x,
        arrowEnd.y - arrowStart.y
    };

    float arrowLength =
        sqrtf(
            arrowDirection.x * arrowDirection.x +
            arrowDirection.y * arrowDirection.y
        );

    arrowDirection.x /= arrowLength;
    arrowDirection.y /= arrowLength;

    // Perpendicular vector used to create the arrowhead.
    Vector2 perpendicular =
    {
        -arrowDirection.y,
        arrowDirection.x
    };

    float arrowHeadLength = 14.0f;
    float arrowHeadWidth = 7.0f;

    Vector2 arrowHeadLeft =
    {
        arrowEnd.x -
            arrowDirection.x * arrowHeadLength +
            perpendicular.x * arrowHeadWidth,

        arrowEnd.y -
            arrowDirection.y * arrowHeadLength +
            perpendicular.y * arrowHeadWidth
    };

    Vector2 arrowHeadRight =
    {
        arrowEnd.x -
            arrowDirection.x * arrowHeadLength -
            perpendicular.x * arrowHeadWidth,

        arrowEnd.y -
            arrowDirection.y * arrowHeadLength -
            perpendicular.y * arrowHeadWidth
    };

    DrawTriangle(
        arrowEnd,
        arrowHeadLeft,
        arrowHeadRight,
        BLACK
    );

    UIText(
        TextFormat("Lift: %+.2f N/m", liftForce),
        static_cast<int>(arrowEnd.x + 12.0f),
        static_cast<int>(arrowEnd.y - 8.0f),
        16,
        BLACK
    );
}

void WindWorld::DrawDragVector()
{
    if (selectedShape != ObstacleShape::Airfoil)
    {
        return;
    }

    // Convert lift force into a visible arrow length.
    float signedArrowLength =
        dragForce * dragVectorPixelsPerNewton;

    signedArrowLength =
        std::clamp(
            signedArrowLength,
            -110.0f,
            110.0f
        );

    // Do not draw an arrowhead when drag is nearly zero.
    if (fabsf(signedArrowLength) < 1.0f)
    {
        DrawCircleV(
            obstaclePosition,
            4.0f,
            BLACK
        );

        return;
    }

    Vector2 arrowStart =
    {
        obstaclePosition.x,
        obstaclePosition.y
    };

    Vector2 arrowEnd =
    {
        //Drag points left
        obstaclePosition.x - signedArrowLength,

        
        obstaclePosition.y
    };

    DrawLineEx(
        arrowStart,
        arrowEnd,
        4.0f,
        BLACK
    );

    // Find the arrow's direction.
    Vector2 arrowDirection =
    {
        arrowEnd.x - arrowStart.x,
        arrowEnd.y - arrowStart.y
    };

    float arrowLength =
        sqrtf(
            arrowDirection.x * arrowDirection.x +
            arrowDirection.y * arrowDirection.y
        );

    arrowDirection.x /= arrowLength;
    arrowDirection.y /= arrowLength;

    // Perpendicular vector used to create the arrowhead.
    Vector2 perpendicular =
    {
        -arrowDirection.y,
        arrowDirection.x
    };

    float arrowHeadLength = 14.0f;
    float arrowHeadWidth = 7.0f;

    Vector2 arrowHeadLeft =
    {
        arrowEnd.x -
            arrowDirection.x * arrowHeadLength +
            perpendicular.x * arrowHeadWidth,

        arrowEnd.y -
            arrowDirection.y * arrowHeadLength +
            perpendicular.y * arrowHeadWidth
    };

    Vector2 arrowHeadRight =
    {
        arrowEnd.x -
            arrowDirection.x * arrowHeadLength -
            perpendicular.x * arrowHeadWidth,

        arrowEnd.y -
            arrowDirection.y * arrowHeadLength -
            perpendicular.y * arrowHeadWidth
    };

    DrawTriangle(
        arrowEnd,
        arrowHeadLeft,
        arrowHeadRight,
        BLACK
    );

    UIText(
        TextFormat("Drag: %.2f N/m", dragForce),
        static_cast<int>(arrowEnd.x - 105.0f),
        static_cast<int>(arrowEnd.y - 8.0f),
        16,
        BLACK
    );
}

//Rendering
//------------
void WindWorld::DrawUI()
{
    UIText("Particle Count", 20, 75, 18, GRAY);

    DrawRectangleRec(particleCountSlider, LIGHTGRAY);

    float sliderAmount =
        static_cast<float>(particleCount - minParticleCount) /
        static_cast<float>(maxParticleCount - minParticleCount);

    float knobX =
        particleCountSlider.x +
        sliderAmount * particleCountSlider.width;

    DrawCircle(
        static_cast<int>(knobX),
        static_cast<int>(
            particleCountSlider.y +
            particleCountSlider.height / 2.0f
        ),
        7.0f,
        DARKGRAY
    );

    UIText(
        TextFormat("%i", particleCount),
        20,
        115,
        18,
        GRAY
    );

    UIText("Wind Speed", 20, 155, 18, GRAY);

    DrawRectangleRec(windSpeedSlider, LIGHTGRAY);

    float windSliderAmount =
        (windSpeed - minWindSpeed) /
        (maxWindSpeed - minWindSpeed);

    float windKnobX =
        windSpeedSlider.x +
        windSliderAmount * windSpeedSlider.width;

    DrawCircle(
        static_cast<int>(windKnobX),
        static_cast<int>(
            windSpeedSlider.y +
            windSpeedSlider.height / 2.0f
        ),
        7.0f,
        DARKGRAY
    );

    UIText(
        TextFormat("%.0f", windSpeed),
        20,
        195,
        18,
        GRAY
    );

    // Particle visualization toggle
    UIText(
        "Visualization",
        20,
        220,
        18,
        GRAY
    );

    auto DrawVisualizationButton =
        [&](Rectangle button,
            const char* text,
            bool selected)
    {
        Color buttonColor =
            selected ? ORANGE : LIGHTGRAY;

        Color textColor =
            selected ? WHITE : DARKGRAY;

        DrawRectangleRounded(
            button,
            0.25f,
            6,
            buttonColor
        );

        DrawRectangleRoundedLines(
            button,
            0.25f,
            6,
            DARKGRAY
        );

        int textWidth = MeasureText(text, 16);

        DrawText(
            text,
            static_cast<int>(
                button.x +
                (button.width - textWidth) / 2.0f
            ),
            static_cast<int>(button.y + 8.0f),
            16,
            textColor
        );
    };

    DrawVisualizationButton(
        speedVisualizationButton,
        "Speed",
        particleVisualization ==
            ParticleVisualization::Speed
    );

    DrawVisualizationButton(
        pressureVisualizationButton,
        "Pressure",
        particleVisualization ==
            ParticleVisualization::Pressure
    );

    // Shape selection
    UIText("Obstacle Shape", 220, 0, 18, DARKGRAY);

    auto DrawShapeButton = [&](Rectangle button,
                            const char* text,
                            bool selected)
    {
        Color buttonColor = selected ? ORANGE : LIGHTGRAY;
        Color textColor = selected ? WHITE : DARKGRAY;

        DrawRectangleRounded(button, 0.25f, 6, buttonColor);
        DrawRectangleRoundedLines(button, 0.25f, 6, DARKGRAY);

        int textWidth = MeasureText(text, 16);

        DrawText(
            text,
            button.x + (button.width - textWidth) / 2,
            button.y + 8,
            16,
            textColor
        );
    };

    DrawShapeButton(
        circleButton,
        "Circle",
        selectedShape == ObstacleShape::Circle);

    DrawShapeButton(
        squareButton,
        "Square",
        selectedShape == ObstacleShape::Square);

    DrawShapeButton(
        diamondButton,
        "Diamond",
        selectedShape == ObstacleShape::Diamond);

    DrawShapeButton(
        airfoilButton,
        "Airfoil",
        selectedShape == ObstacleShape::Airfoil);


    if (selectedShape == ObstacleShape::Airfoil)
    {
        UIText(
            "Angle of Attack",
            20,
            300,
            18,
            GRAY
        );

        DrawRectangleRec(
            angleSlider,
            LIGHTGRAY
        );

        float angleSliderAmount =
            (airfoilAngleDegrees - minAngle) /
            (maxAngle - minAngle);

        float angleKnobX =
            angleSlider.x +
            angleSliderAmount *
            angleSlider.width;

        DrawCircle(
            static_cast<int>(angleKnobX),
            static_cast<int>(
                angleSlider.y +
                angleSlider.height / 2.0f
            ),
            7.0f,
            DARKGRAY
        );

        UIText(
            TextFormat(
                "%+.1f degrees",
                airfoilAngleDegrees
            ),
            20,
            365,
            18,
            DARKGRAY
        );

        UIText(
            TextFormat(
                "Lift Coefficient: %+.2f",
                liftCoefficient
            ),
            20,
            395,
            17,
            DARKGRAY
        );

        UIText(
            TextFormat(
                "Lift: %+.2f N/m",
                liftForce
            ),
            20,
            420,
            17,
            DARKGRAY
        );

        UIText(
            TextFormat(
                "Drag Coefficient: %+.2f",
                dragCoefficient
            ),
            20,
            465,
            17,
            DARKGRAY
        );

        UIText(
            TextFormat(
                "Drag: %.2f N/m",
                dragForce
            ),
            20,
            490,
            17,
            DARKGRAY
        );
    }

    
        
    UIText("R: Reset", 20, 20, 18, GRAY);
    UIText("Backspace: Menu", 20, 570, 18, GRAY);
}

void WindWorld::DrawParticles()
{
    for (const WindParticle& particle : particles)
    {
        float particleSpeed = Vector2Length(particle.velocity);
        float referenceSpeed = Vector2Length(velocity);
        
        float pressureDifference =
            0.5f * airDensity *
            (referenceSpeed * referenceSpeed -
            particleSpeed * particleSpeed);

        float pressureScale = 5000.0f;

        float pressureAmount =
            std::clamp(pressureDifference / pressureScale, 
            -1.0f,
             1.0f
        );

        float dx =
            particle.position.x - obstaclePosition.x;

        float dy =
            particle.position.y - obstaclePosition.y;

        float distanceFromAirfoil =
            sqrtf(dx * dx + dy * dy);

        float pressureRadius = 180.0f;

        float pressureWeight = std::clamp(
            1.0f - distanceFromAirfoil / pressureRadius,
            0.0f,
            1.0f
        );

        pressureAmount *= pressureWeight;

        if (particle.position.x > obstaclePosition.x)
        {
            float downstreamDistance =
                particle.position.x - obstaclePosition.x;

            float wakeFade = std::clamp(
                1.0f - downstreamDistance / 120.0f,
                0.0f,
                1.0f
            );

            pressureAmount *= wakeFade;
        }


        float speed = sqrt(
            particle.velocity.x * particle.velocity.x +
            particle.velocity.y * particle.velocity.y
        );

        float referenceWindSpeed = sqrt(
            velocity.x * velocity.x +
            velocity.y * velocity.y
        );

        float minDisplaySpeed = windSpeed * 0.97f;
        float maxDisplaySpeed = windSpeed * 1.03f;

        float speedAmount = (speed - minDisplaySpeed) / 
                            (maxDisplaySpeed - minDisplaySpeed);

        speedAmount = std::clamp(speedAmount, 0.0f, 1.0f);

        Color particleColor;

        if (particleVisualization == 
            ParticleVisualization::Speed)
        {
            if (speedAmount < 0.5f)
            {
                float t = speedAmount / 0.5f;
                particleColor = ColorLerp(BLUE, GREEN, t);
            }
            else
            {
                float t = (speedAmount - 0.5f) / 0.5f;
                particleColor = ColorLerp(GREEN, RED, t);
            }
        }
        else
        {
            float angleRadians =
                airfoilAngleDegrees * DEG2RAD;

            Vector2 localPosition =
                AirfoilWorldToLocal(particle.position, obstaclePosition, angleRadians);

            float chordDistance =
                fabsf(localPosition.x);

            float surfaceDistance =
                fabsf(localPosition.y);

            float chord = obstacleHalfSize * 3.0f;

            float normalizedChord =
                (localPosition.x / chord) + 0.5f;

            float chordWeight = std::clamp(
                1.0f - normalizedChord,
                0.0f,
                1.0f
            );

            float surfaceWeight = std::clamp(
                1.0f - surfaceDistance / 80.0f,
                0.0f,
                1.0f
            );

            pressureAmount *=
                chordWeight *
                surfaceWeight;

            if (pressureAmount > 0.0f)
            {
                particleColor = ColorLerp(
                    LIGHTGRAY,
                    RED,
                    pressureAmount
                );
            }
            else
            {
                particleColor = ColorLerp(
                    LIGHTGRAY,
                    BLUE,
                    -pressureAmount
                );
            }
        }

        //Draw Trials in either visualization mode
        for (size_t i = 1; i < particle.trail.size(); i++)
        {
            float alpha =
                static_cast<float>(i) /
                static_cast<float>(particle.trail.size());

            DrawLineEx(
                particle.trail[i - 1],
                particle.trail[i],
                1.0f,
                Fade(particleColor, alpha)
            );
        }
       
        

        DrawCircleV(
            particle.position,
            particle.radius,
            particleColor
        );
    }
}

void WindWorld::DrawCircleObstacle()
{
     DrawCircleV(
            obstaclePosition,
            obstacleHalfSize,
            obstacleColor
        );
}

void WindWorld::DrawSquareObstacle()
{
    DrawRectanglePro(
            {
                obstaclePosition.x,
                obstaclePosition.y,
                obstacleHalfSize * 2.0f,
                obstacleHalfSize * 2.0f
            },
            {
                obstacleHalfSize,
                obstacleHalfSize
            },
            0.0f,
            obstacleColor
        );
}

void WindWorld::DrawDiamondObstacle()
{
    DrawRectanglePro(
            {
                obstaclePosition.x,
                obstaclePosition.y,
                obstacleHalfSize * 2.0f,
                obstacleHalfSize * 2.0f
            },
            {
                obstacleHalfSize,
                obstacleHalfSize
            },
            45.0f,
            obstacleColor
        );
}

void WindWorld::DrawAirfoilObstacle()
{
        const int segmentCount = 32;

        float chord = obstacleHalfSize * 3.0f;
        float thicknessRatio = 0.12f;
        float airfoilAngleRadians =
            airfoilAngleDegrees * DEG2RAD;


        for (int i = 0; i < segmentCount; i++)
        {
            float x1 = static_cast<float>(i) /
                    static_cast<float>(segmentCount);

            float x2 = static_cast<float>(i + 1) /
                    static_cast<float>(segmentCount);

            float thickness1 =
                5.0f * thicknessRatio * chord *
                (
                    0.2969f * sqrtf(x1) -
                    0.1260f * x1 -
                    0.3516f * x1 * x1 +
                    0.2843f * x1 * x1 * x1 -
                    0.1036f * x1 * x1 * x1 * x1
                );

            float thickness2 =
                5.0f * thicknessRatio * chord *
                (
                    0.2969f * sqrtf(x2) -
                    0.1260f * x2 -
                    0.3516f * x2 * x2 +
                    0.2843f * x2 * x2 * x2 -
                    0.1036f * x2 * x2 * x2 * x2
                );

            Vector2 upperStartLocal =
            {
                -chord * 0.5f + x1 * chord,
                -thickness1
            };

            Vector2 upperEndLocal =
            {
                -chord * 0.5f + x2 * chord,
                -thickness2
            };

            Vector2 lowerStartLocal =
            {
                -chord * 0.5f + x1 * chord,
                thickness1
            };

            Vector2 lowerEndLocal =
            {
                -chord * 0.5f + x2 * chord,
                thickness2
            };

            Vector2 upperStart =
                AirfoilLocalToWorld(
                    upperStartLocal,
                    obstaclePosition,
                    airfoilAngleRadians
                );

            Vector2 upperEnd =
                AirfoilLocalToWorld(
                    upperEndLocal,
                    obstaclePosition,
                    airfoilAngleRadians
                );

            Vector2 lowerStart =
                AirfoilLocalToWorld(
                    lowerStartLocal,
                    obstaclePosition,
                    airfoilAngleRadians
                );

            Vector2 lowerEnd =
                AirfoilLocalToWorld(
                    lowerEndLocal,
                    obstaclePosition,
                    airfoilAngleRadians
                );

            DrawTriangle(
                upperStart,
                lowerStart,
                upperEnd,
                obstacleColor
            );

            DrawTriangle(
                upperEnd,
                lowerStart,
                lowerEnd,
                obstacleColor
            );
        }

}

void WindWorld::Draw()
{
    ClearBackground(Color{ 225, 235, 245, 255});

    DrawRectangleLinesEx(tunnel, 8.0f, LIGHTGRAY);

    DrawParticles();

    switch (selectedShape)
    {
        case ObstacleShape::Circle:
        {
            DrawCircleObstacle();
            break;
        }

        case ObstacleShape::Square:
        {
            DrawSquareObstacle();
            break;
        }

        case ObstacleShape::Diamond:
        {
            DrawDiamondObstacle();
            break;
        }

        case ObstacleShape::Airfoil:
        {
            DrawAirfoilObstacle();
            break;
        }
    }
    DrawLiftVector();
    DrawDragVector();
    DrawUI();
}