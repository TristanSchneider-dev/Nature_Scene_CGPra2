#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float WaveHeight; // NEU: Input vom Vertex Shader

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float time;

// Hash & Noise für die kleinen Details
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i + vec2(0.0, 0.0));
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main() {
    vec3 lPos = (lightPos == vec3(0.0)) ? vec3(50.0, 100.0, 50.0) : lightPos;
    vec3 lColor = (lightColor == vec3(0.0)) ? vec3(1.0) : lightColor;

    // --- Noise Perturbation (Das "Kribbeln") ---
    vec2 noiseCoord1 = (FragPos.xz + time * 0.5) * 15.0;
    vec2 noiseCoord2 = (FragPos.xz - time * 0.7) * 25.0;
    float n1 = noise(noiseCoord1);
    float n2 = noise(noiseCoord2);
    vec3 noisePerturbation = vec3(n1 - 0.5, 0.0, n2 - 0.5);
    float noiseStrength = 0.3; // Etwas reduziert für ruhigeres Wasser
    vec3 finalNormal = normalize(Normal + noisePerturbation * noiseStrength);

    // --- Beleuchtung ---
    // Ambient
    vec3 ambient = 0.1 * vec3(0.05, 0.1, 0.15);

    // Diffuse
    vec3 norm = finalNormal;
    vec3 lightDir = normalize(lPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    // Dunkle Basis-Farbe (Deep Ocean Blue)
    vec3 baseColor = vec3(0.05, 0.1, 0.2);
    vec3 diffuse = diff * lColor * baseColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lColor * 1.5;

    // Fresnel
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 4.0);
    vec3 rimColor = vec3(0.5, 0.7, 0.8) * fresnel;

    // --- NEU: Schaum (Foam) Berechnung ---
    // Wir prüfen, ob die Wellenhöhe über einem gewissen Wert liegt.
    // Die Werte 0.4 und 0.6 steuern, ab welcher Höhe der Schaum beginnt und wie hart der Übergang ist.
    // noise(FragPos.xz * 3.0) fügt dem Schaumrand etwas Unregelmäßigkeit hinzu.
    float foamMask = smoothstep(0.45, 0.7, WaveHeight + noise(FragPos.xz * 2.0) * 0.1);
    vec3 foamColor = vec3(0.95); // Fast Weiß

    // Zusammenbauen
    vec3 waterResult = ambient + diffuse + specular + (rimColor * 0.5);

    // Schaum drübermischen
    vec3 finalResult = mix(waterResult, foamColor, foamMask);

    // Fog
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-0.025 * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 fogColor = vec3(0.5, 0.6, 0.7);

    FragColor = vec4(mix(fogColor, finalResult, fogFactor), 1.0);
}