#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

out vec2 TexCoords;
out float GrassHeight; // 0.0 = Boden, 1.0 = Spitze
out vec3 WorldPos;     // Für Farbvariation

uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    TexCoords = aTexCoords;
    GrassHeight = aPos.y; // Da dein Quad von 0.0 bis 1.0 in Y geht

    vec3 pos = aPos;

    // Wind Animation (Behalten wir bei)
    if(pos.y > 0.1) {
        float noise = aInstanceMatrix[3][0] * 0.5 + aInstanceMatrix[3][2] * 0.5;
        float wave = sin(time * 2.0 + noise);
        pos.x += wave * 0.1 * pos.y; // * pos.y damit es unten fest bleibt
        pos.z += wave * 0.05 * pos.y;
    }

    vec4 worldPosition = aInstanceMatrix * vec4(pos, 1.0);
    WorldPos = worldPosition.xyz;
    gl_Position = projection * view * worldPosition;
}