#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
// NEU: Wir geben die lokale Höhe der Welle weiter für den Schaum
out float WaveHeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

uniform float speedMult;
uniform float steepnessMult;
uniform float wavelengthMult;

#define NUM_WAVES 8

struct WaveParams {
    vec2 dir;
    float steepness;
    float wavelength;
};

vec2 rotate(vec2 v, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

vec3 calculateSingleWave(WaveParams w, vec3 p, inout vec3 tangent, inout vec3 binormal) {
    float finalWavelength = w.wavelength * wavelengthMult;
    finalWavelength = max(finalWavelength, 0.001);

    float k = 2.0 * 3.14159 / finalWavelength;
    float c = sqrt(9.8 / k);
    vec2 d = normalize(w.dir);

    float f = k * (dot(d, p.xz) - c * time * speedMult);
    float a = (w.steepness * steepnessMult) / k;

    float sinf = sin(f);
    float cosf = cos(f);

    float wa = k * a;

    tangent += vec3(
    -d.x * d.x * (wa * sinf),
    d.x * (wa * cosf),
    -d.x * d.y * (wa * sinf)
    );

    binormal += vec3(
    -d.x * d.y * (wa * sinf),
    d.y * (wa * cosf),
    -d.y * d.y * (wa * sinf)
    );

    return vec3(
    d.x * (a * cosf),
    a * sinf,
    d.y * (a * cosf)
    );
}

void main() {
    vec3 gridPos = aPos;
    vec3 finalPos = gridPos;
    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);

    vec3 offset = vec3(0.0);

    float currentWavelength = 15.0;
    float currentSteepness = 0.25;
    vec2 currentDir = vec2(1.0, 0.5);

    for(int i = 0; i < NUM_WAVES; i++) {
        WaveParams w;
        w.dir = currentDir;
        w.wavelength = currentWavelength;
        w.steepness = currentSteepness;

        offset += calculateSingleWave(w, gridPos, tangent, binormal);

        currentDir = rotate(currentDir, radians(55.0 + float(i) * 12.0));
        currentWavelength *= 0.62;
        currentSteepness *= 0.82;
    }

    finalPos += offset;

    // NEU: Wir speichern, wie hoch die Welle an dieser Stelle ist
    WaveHeight = offset.y;

    Normal = normalize(cross(binormal, tangent));
    FragPos = vec3(model * vec4(finalPos, 1.0));
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}