#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D mapAlbedo;
uniform sampler2D mapNormal;
uniform sampler2D mapARM; // R=AO, G=Roughness, B=Metallic

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform bool useNormalMap;
uniform bool useARMMap;

void main()
{
    vec4 albedoSample = texture(mapAlbedo, TexCoords);

    // [FIX 1] Input Gamma Korrektur:
    // Wir wandeln die sRGB Textur manuell in Linear Space um, damit das Licht korrekt berechnet wird.
    // Das macht die Textur im "Rechenraum" dunkler und realistischer.
    vec3 color = pow(albedoSample.rgb, vec3(2.2));

    vec3 norm = normalize(Normal);
    if(useNormalMap) {
        vec3 normalMapValue = texture(mapNormal, TexCoords).rgb;
        norm = normalize(normalMapValue * 2.0 - 1.0);
        norm = normalize(TBN * norm);
    }

    float ao = 1.0;
    float roughness = 0.8; // Standard etwas rauer, falls keine Map da ist
    float metallic = 0.0;

    if(useARMMap) {
        vec3 arm = texture(mapARM, TexCoords).rgb;
        ao = arm.r;
        roughness = arm.g;
        metallic = arm.b; // Metallic auslesen!
    }

    // [FIX 2] Ambient verringern
    vec3 ambient = 0.03 * color * ao;

    vec3 lightDir = normalize(lightPos - WorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;

    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float specPower = (1.0 - roughness) * 64.0;
    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(specPower, 0.001));

    // Einfache Specular Berechnung (Nicht physikalisch perfekt, aber okay für jetzt)
    vec3 specular = vec3(0.5) * spec * (1.0 - roughness);

    vec3 result = ambient + diffuse + specular;

    // [FIX 3] Output Gamma entfernen!
    // Da glEnable(GL_FRAMEBUFFER_SRGB) an ist, macht OpenGL das automatisch.
    // Wir geben hier das LINEARE Ergebnis aus.
    FragColor = vec4(result, 1.0);
}