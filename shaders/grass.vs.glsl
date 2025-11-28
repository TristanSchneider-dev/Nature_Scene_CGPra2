#version 330 core
layout (location = 0) in vec3 aPos;      // Mesh Pos
layout (location = 1) in vec2 aTexCoord; // UV
layout (location = 2) in vec3 aInstancePos; // Instanz Position
layout (location = 3) in vec3 aInstanceNormal; // Instanz Normale (vom Terrain)
layout (location = 4) in float aInstanceDryness; // Dryness

out vec2 TexCoords;
out float Height;
out float Dryness;

uniform mat4 view;
uniform mat4 projection;

// Zufallsgenerator
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// Rotation um Y (für zufällige Drehung)
mat4 rotateY(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat4(c, 0, -s, 0,  0, 1, 0, 0,  s, 0, c, 0,  0, 0, 0, 1);
}

// Ausrichtung an Normale
// Wir konstruieren eine Matrix, die (0,1,0) auf 'normal' abbildet.
mat3 alignToNormal(vec3 normal) {
    vec3 target = normalize(normal);
    // Ein Hilfsvektor, der nicht parallel zur Normalen ist
    vec3 helper = vec3(0, 0, 1);
    if (abs(target.z) > 0.99) helper = vec3(1, 0, 0);

    vec3 tangent = normalize(cross(target, helper));
    vec3 bitangent = normalize(cross(tangent, target));

    // Matrix konstruieren (Tangent, Normal, Bitangent)
    // Da Gras nach Y wächst, muss 'target' in die 2. Spalte (Y).
    return mat3(tangent, target, bitangent);
}

void main()
{
    vec3 localPos = aPos;

    // 1. Zufällige Rotation um die eigene Y-Achse
    // Damit sich das Gras nicht immer gleich dreht
    float rndRot = random(aInstancePos.xz) * 6.28;
    localPos = (rotateY(rndRot) * vec4(localPos, 1.0)).xyz;

    // 2. Ausrichtung an die Terrain-Normale
    // Wir drehen das lokale System so, dass Y in Richtung der Normalen zeigt.
    mat3 alignMat = alignToNormal(aInstanceNormal);
    localPos = alignMat * localPos;

    // 3. Weltposition addieren
    vec3 worldPos = localPos + aInstancePos;

    TexCoords = aTexCoord;
    Height = aTexCoord.y;
    Dryness = aInstanceDryness;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}