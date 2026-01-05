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

    // [FIX] Transparenz-Cutoff:
    // Wenn der Pixel fast durchsichtig ist (z.B. der Rand vom Blatt), wird er nicht gezeichnet.
    if(albedoSample.a < 0.1)
        discard;

    // Input Gamma Korrektur (sRGB zu Linear)
    vec3 color = pow(albedoSample.rgb, vec3(2.2));

    vec3 norm = normalize(Normal);
    if(useNormalMap) {
        vec3 normalMapValue = texture(mapNormal, TexCoords).rgb;
        norm = normalize(normalMapValue * 2.0 - 1.0);
        norm = normalize(TBN * norm);
    }

    float ao = 1.0;
    float roughness = 0.8; // Standard etwas rauer
    float metallic = 0.0;

    if(useARMMap) {
        vec3 arm = texture(mapARM, TexCoords).rgb;
        ao = arm.r;
        roughness = arm.g;
        metallic = arm.b;
    }

    // Ambient
    vec3 ambient = 0.03 * color * ao;

    // Diffuse
    vec3 lightDir = normalize(lightPos - WorldPos);
    float diff = abs(dot(norm, lightDir));
    diff = max(diff, 0.2);
    vec3 diffuse = diff * lightColor * color;

    // Specular
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specPower = (1.0 - roughness) * 64.0;
    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(specPower, 0.001));
    vec3 specular = vec3(0.5) * spec * (1.0 - roughness);

    vec3 result = ambient + diffuse + specular;

    // Output (Linear, da GL_FRAMEBUFFER_SRGB aktiviert ist)
    FragColor = vec4(result, 1.0);
}