#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Height;
in float Dryness;

uniform sampler2D grassTexture;
uniform vec3 colorHealthy; // Grün / Dunkelbraun
uniform vec3 colorDry;     // Oliv / Hellbraun

void main()
{
    vec4 texColor = texture(grassTexture, TexCoords);

    // Alpha Test
    if(texColor.a < 0.1)
        discard;

    // 1. Farbe mischen (Gesund vs Trocken) basierend auf Untergrund
    vec3 baseColor = mix(colorHealthy, colorDry, Dryness);

    // 2. Oben (Spitze) leicht heller/gelblicher machen
    vec3 tipTint = vec3(0.1, 0.1, 0.0) * Height;
    baseColor += tipTint;

    // 3. Unten abdunkeln (Fake Ambient Occlusion)
    baseColor *= (0.5 + 0.5 * Height);

    FragColor = vec4(baseColor, 1.0);
}