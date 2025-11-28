#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aInstance;

out vec2 TexCoords;
out float Height;
out float Dryness;

uniform mat4 view;
uniform mat4 projection;

// Rotation um Y-Achse
mat4 rotateY(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat4(
        c, 0, -s, 0,
        0, 1,  0, 0,
        s, 0,  c, 0,
        0, 0,  0, 1
    );
}

// Zufallsgenerator
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main()
{
    vec3 localPos = aPos;
    vec3 instancePos = aInstance.xyz;

    // --- KEIN TAPERING MEHR (Gerade Halme) ---
    // Die Form kommt nur aus der Textur (Transparenz).

    // Zufällige Rotation
    float rndRot = random(instancePos.xz) * 6.28;
    localPos = (rotateY(rndRot) * vec4(localPos, 1.0)).xyz;

    vec3 worldPos = localPos + instancePos;

    TexCoords = aTexCoord;
    Height = aTexCoord.y;
    Dryness = aInstance.w;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}