#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 VertexColor;
in mat3 TBN;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

// --- Flags für Effekte ---
uniform bool useNormalMap;
uniform bool useARMMap;

// Textur Sampler (wie gehabt)
uniform sampler2D texGravelDiff;
uniform sampler2D texGravelNor;
uniform sampler2D texGravelArm;

uniform sampler2D texPebblesDiff;
uniform sampler2D texPebblesNor;
uniform sampler2D texPebblesArm;

uniform sampler2D texRockDiff;
uniform sampler2D texRockNor;
uniform sampler2D texRockArm;

vec3 getNormalFromMap(sampler2D normalMap, vec2 uv) {
    vec3 tangentNormal = texture(normalMap, uv).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

void main()
{
    float tiling = 20.0;
    vec2 tiledCoords = TexCoords * tiling;

    // --- 1. Albedo (Farbe) immer laden ---
    vec3 dGravel  = texture(texGravelDiff,  tiledCoords).rgb;
    vec3 dPebbles = texture(texPebblesDiff, tiledCoords).rgb;
    vec3 dRock    = texture(texRockDiff,    tiledCoords).rgb;

    vec3 finalAlbedo = dGravel;
    finalAlbedo = mix(finalAlbedo, dPebbles, VertexColor.r);
    finalAlbedo = mix(finalAlbedo, dRock,    VertexColor.b);

    // --- 2. Normal Mapping Logik ---
    vec3 finalNormal;
    if (useNormalMap) {
        vec3 nGravel  = getNormalFromMap(texGravelNor,  tiledCoords);
        vec3 nPebbles = getNormalFromMap(texPebblesNor, tiledCoords);
        vec3 nRock    = getNormalFromMap(texRockNor,    tiledCoords);

        finalNormal = nGravel;
        finalNormal = mix(finalNormal, nPebbles, VertexColor.r);
        finalNormal = mix(finalNormal, nRock,    VertexColor.b);
        finalNormal = normalize(finalNormal);
    } else {
        // Fallback: Die geometrische Normale nutzen (3. Spalte der TBN Matrix)
        finalNormal = normalize(TBN[2]);
    }

    // --- 3. ARM / PBR Logik ---
    float ao = 1.0;
    float roughness = 1.0; // Standard: Matt
    float metallic = 0.0;

    if (useARMMap) {
        vec3 armGravel  = texture(texGravelArm,  tiledCoords).rgb;
        vec3 armPebbles = texture(texPebblesArm, tiledCoords).rgb;
        vec3 armRock    = texture(texRockArm,    tiledCoords).rgb;

        vec3 finalARM = armGravel;
        finalARM = mix(finalARM, armPebbles, VertexColor.r);
        finalARM = mix(finalARM, armRock,    VertexColor.b);

        ao = finalARM.r;
        roughness = finalARM.g;
        metallic = finalARM.b;
    }

    // --- 4. Lighting Calculation ---
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // Ambient
    vec3 ambient = vec3(0.1) * finalAlbedo * ao;

    // Diffuse
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * finalAlbedo;

    // Specular
    // Shininess basierend auf Roughness (oder Standardwert wenn ARM aus)
    float shininess = mix(2.0, 64.0, (1.0 - roughness));

    float spec = 0.0;
    if(diff > 0.0) {
        spec = pow(max(dot(finalNormal, halfwayDir), 0.0), shininess);
    }

    // Specular Intensity dämpfen (Wet-Look Fix aus vorigem Schritt)
    float specIntensity = (1.0 - roughness) * 0.3;

    vec3 specularColor = mix(vec3(1.0), finalAlbedo, metallic);
    vec3 specular = spec * specularColor * lightColor * specIntensity;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}