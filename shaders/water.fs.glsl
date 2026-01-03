#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float WaveHeight;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float time;
// NEU: Nebelfarbe muss von außen kommen, damit sie zur Nacht passt!
uniform vec3 fogColor;

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

    float noiseStrength = 0.3;
    vec3 finalNormal = normalize(Normal + noisePerturbation * noiseStrength);

    // --- Beleuchtung ---

    // FIX 1: Ambient muss auf Licht reagieren (zumindest etwas), sonst leuchtet es grau
    // Wir nehmen die Helligkeit des Lichts oder multiplizieren direkt
    vec3 ambientBase = vec3(0.05, 0.1, 0.15);
    // Wir mischen es etwas mit lColor, damit es nachts blau wird
    vec3 ambient = 0.1 * ambientBase * (lColor + 0.2);

    // Diffuse
    vec3 norm = finalNormal;
    vec3 lightDir = normalize(lPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 baseColor = vec3(0.05, 0.1, 0.2);
    vec3 diffuse = diff * lColor * baseColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lColor * 1.5;

    // Fresnel / Rim
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 4.0);
    // FIX 2: Rim-Color mit lColor multiplizieren!
    vec3 rimColor = vec3(0.5, 0.7, 0.8) * fresnel * lColor;

    // --- Schaum (Foam) Berechnung ---
    float foamMask = smoothstep(0.45, 0.7, WaveHeight + noise(FragPos.xz * 2.0) * 0.1);

    // FIX 3: Schaum darf nachts nicht weiß leuchten -> mit lColor tönen
    vec3 foamBase = vec3(0.95);
    vec3 foamColor = foamBase * lColor;

    // Zusammenbauen
    vec3 waterResult = ambient + diffuse + specular + (rimColor * 0.5);

    // Schaum drübermischen
    vec3 finalResult = mix(waterResult, foamColor, foamMask);

    // Fog
    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-0.025 * dist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // FIX 4: Nutze die Uniform statt hardcoded Grau
    FragColor = vec4(mix(fogColor, finalResult, fogFactor), 1.0);
}