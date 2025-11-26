#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Wird hier ignoriert, wir berechnen neu

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// --- Noise Logik ---
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float getTerrainHeight(vec2 pos) {
    float freq = 0.3;
    float amp = 2.0;
    float h = noise(pos * freq) * amp;
    h += noise(pos * freq * 2.0) * (amp * 0.5);
    return h;
}

void main()
{
    // Weltposition berechnen
    vec4 worldPos = model * vec4(aPos, 1.0);

    // Höhe anwenden
    float h = getTerrainHeight(worldPos.xz);
    worldPos.y += h;

    FragPos = vec3(worldPos);

    // Normalen neu berechnen (Finite Difference)
    float e = 0.01;
    float hL = getTerrainHeight(worldPos.xz - vec2(e, 0.0));
    float hR = getTerrainHeight(worldPos.xz + vec2(e, 0.0));
    float hD = getTerrainHeight(worldPos.xz - vec2(0.0, e));
    float hU = getTerrainHeight(worldPos.xz + vec2(0.0, e));

    vec3 newNormal;
    newNormal.x = hL - hR;
    newNormal.y = 2.0 * e;
    newNormal.z = hD - hU;
    Normal = normalize(newNormal);

    gl_Position = projection * view * worldPos;
}