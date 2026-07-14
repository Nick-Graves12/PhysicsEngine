#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main()
{
    vec4 sampleColor =
        texture(texture0, fragTexCoord);

    float density = sampleColor.a;

    // Controls where the fluid becomes visible.
    float body = smoothstep(
        0.05,
        0.20,
        density
    );

    // Controls the gradual color transition.
    float brightness = smoothstep(
        0.02,
        0.90,
        density
    );

    brightness = pow(
        brightness,
        1.8
    );

    vec3 deepWater = vec3(
        0.00,
        0.20,
        0.58
    );

    vec3 brightWater = vec3(
        0.05,
        0.82,
        1.00
    );

    // Render textures are vertically flipped in your DrawTexturePro call,
    // so this may need to be 1.0 - fragTexCoord.y depending on appearance.
    float depthFactor = fragTexCoord.y;

    float colorAmount =
        brightness * 0.90 +
        (1.0 - depthFactor) * 0.10;

    colorAmount *=
        1.0 - depthFactor * 0.12;

    colorAmount = clamp(
        colorAmount,
        0.0,
        1.0
    );

    vec3 waterColor = mix(
        deepWater,
        brightWater,
        colorAmount
    );

    float surface =
        smoothstep(
            0.16,
            0.22,
            density
    )
    -
    smoothstep(
        0.22,
        0.28,
        density
    );

    waterColor += surface * 0.08;

    finalColor = vec4(
        waterColor,
        body * 0.75
    );
}