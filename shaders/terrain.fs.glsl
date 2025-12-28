#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} fs_in;

// Texturen
uniform sampler2D pebblesAlbedo;
uniform sampler2D pebblesNormal;
uniform sampler2D pebblesARM;

uniform sampler2D groundAlbedo;
uniform sampler2D groundNormal;
uniform sampler2D groundARM;

uniform sampler2D rockAlbedo;
uniform sampler2D rockNormal;
uniform sampler2D rockARM;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float tiling;

vec3 getNormalFromMap(sampler2D normalMap, vec2 uv) {
    vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - 1.0;
    return normalize(fs_in.TBN * tangentNormal);
}

void main()
{
    vec2 uv = fs_in.TexCoords * tiling;

    // --- 1. Mix-Faktoren ---
    float slope = dot(normalize(fs_in.Normal), vec3(0.0, 1.0, 0.0));
    float height = fs_in.FragPos.y;

    float pebblesWeight = 1.0 - smoothstep(-2.5, -2.0, height);
    float rockWeight = 1.0 - smoothstep(0.5, 0.8, slope);
    float groundWeight = 1.0 - max(pebblesWeight, rockWeight);

    float total = pebblesWeight + rockWeight + groundWeight;
    pebblesWeight /= total; rockWeight /= total; groundWeight /= total;

    // --- 2. Sampling ---
    // HINWEIS: Wir multiplizieren albedo leicht mit einer Farbe, um sie satter zu machen,
    // oder wir machen es ganz unten global.
    vec3 colPebbles = texture(pebblesAlbedo, uv).rgb;
    vec3 colGround  = texture(groundAlbedo, uv).rgb;
    vec3 colRock    = texture(rockAlbedo, uv).rgb;

    vec3 nPebbles = getNormalFromMap(pebblesNormal, uv);
    vec3 nGround  = getNormalFromMap(groundNormal, uv);
    vec3 nRock    = getNormalFromMap(rockNormal, uv);

    vec3 armPebbles = texture(pebblesARM, uv).rgb;
    vec3 armGround  = texture(groundARM, uv).rgb;
    vec3 armRock    = texture(rockARM, uv).rgb;

    // --- 3. Blending ---
    vec3 albedo = colPebbles * pebblesWeight + colGround * groundWeight + colRock * rockWeight;
    vec3 normal = normalize(nPebbles * pebblesWeight + nGround * groundWeight + nRock * rockWeight);
    vec3 arm    = armPebbles * pebblesWeight + armGround * groundWeight + armRock * rockWeight;

    float ao = arm.r;
    float roughness = arm.g;
    float metallic = arm.b;

    // --- 4. Beleuchtung ---
    vec3 ambient = 0.1 * albedo * ao; // Ambient etwas erhöht für sattere Schatten

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

    // Specular etwas reduzieren (0.5 * ...), damit es nicht so "plastikartig" weiß wirkt
    float specPower = mix(64.0, 2.0, roughness);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specPower);
    vec3 specular = vec3(spec) * metallic * lightColor * 0.5;

    vec3 result = ambient + diffuse + specular;

    // --- 5. Post-Processing Tweaks ---

    // Sättigung erhöhen (Saturation Boost)
    float saturation = 1.3; // Wert > 1.0 macht es bunter, < 1.0 macht es grauer
    vec3 gray = vec3(dot(result, vec3(0.299, 0.587, 0.114)));
    result = mix(gray, result, saturation);

    // WICHTIG: Keine manuelle Gamma-Korrektur hier, da GL_FRAMEBUFFER_SRGB in main.cpp an ist!
    // result = pow(result, vec3(1.0/2.2)); <--- DAS WAR DER ÜBELTÄTER

    FragColor = vec4(result, 1.0);
}