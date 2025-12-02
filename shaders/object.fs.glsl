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
    // [FIX] Discard entfernt, da Steine solide sind!
    // if(albedoSample.a < 0.5) discard;

    vec3 color = albedoSample.rgb;

    vec3 norm = normalize(Normal);
    if(useNormalMap) {
        vec3 normalMapValue = texture(mapNormal, TexCoords).rgb;
        // Normal Map von [0,1] auf [-1,1] bringen
        norm = normalize(normalMapValue * 2.0 - 1.0);
        norm = normalize(TBN * norm);
    }

    float ao = 1.0;
    float roughness = 0.5;

    if(useARMMap) {
        vec3 arm = texture(mapARM, TexCoords).rgb;
        ao = arm.r;
        roughness = arm.g;
    }

    vec3 ambient = 0.1 * color * ao; // Etwas Ambient Licht ist immer gut

    vec3 lightDir = normalize(lightPos - WorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;

    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float specPower = (1.0 - roughness) * 64.0;
    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(specPower, 0.001));
    vec3 specular = vec3(0.5) * spec * (1.0 - roughness);

    vec3 result = ambient + diffuse + specular;

    // Gamma Correction
    result = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0);
}