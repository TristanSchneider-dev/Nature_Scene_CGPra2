#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float GrassHeight; // Kommt vom Vertex Shader
in vec3 WorldPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Simple Noise Funktion für Farbvariation
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // 1. Alpha Test
    if(texColor.a < 0.2)
        discard;

    // 2. FAKE AMBIENT OCCLUSION (AO)
    // Macht das Gras unten dunkler. Das bringt extrem viel Volumen!
    // Mixe zwischen einer dunklen Bodenfarbe (schwarz/braun) und der Textur
    float aoFactor = pow(GrassHeight, 0.5); // Nicht linear, sondern Kurve
    vec3 objectColor = texColor.rgb * aoFactor;

    // 3. FARB-VARIATION (Damit nicht jeder Halm gleich aussieht)
    // Wir nutzen die Weltposition, um manche Halme etwas gelber/dunkler zu machen

    float noiseVal1 = random(WorldPos.xz * 0.1);  // Groß
    float noiseVal2 = random(WorldPos.xz * 0.5);  // Mittel
    float noiseVal = noiseVal1 * 0.5 + noiseVal2 * 0.3;
    vec3 variance = vec3(0.1, 0.1, 0.0) * (noiseVal - 0.5); // Leichtes Rauschen
    objectColor += variance;

    // 4. BELEUCHTUNG (Soft Lighting)
    // Trick: Wir nutzen als Normale einfach (0,1,0) - also nach oben.
    // Das verhindert, dass Grasflächen schwarz werden, wenn sie von der Sonne weggedreht sind.
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 lightDir = normalize(lightPos - WorldPos);

    // Diffus
    float diff = max(dot(normal, lightDir), 0.4); // Mindestens 0.4 Helligkeit (Fake Translucency)
    vec3 diffuse = diff * lightColor;

    // Ambient
    vec3 ambient = 0.4 * lightColor;

    // Final Mix
    vec3 finalColor = (ambient + diffuse) * objectColor;

    // Nebel-Support (optional, falls du Nebel nutzt)
    // mix(finalColor, fogColor, fogFactor)... hier weggelassen für Übersicht

    FragColor = vec4(finalColor, texColor.a);
}