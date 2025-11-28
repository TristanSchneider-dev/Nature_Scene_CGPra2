#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float zNear;
uniform float zFar;
uniform vec3 fogColor;
uniform float density; // NEU: Wird jetzt von der CPU gesteuert

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main() {
    vec3 col = texture(screenTexture, TexCoords).rgb;
    float depthValue = texture(depthTexture, TexCoords).r;
    float dist = LinearizeDepth(depthValue);

    // Exponential Squared Fog
    // Wenn density 0.0 ist (Toggle aus), ist fogFactor 0.0 -> Kein Nebel
    float fogFactor = 1.0 - exp(-pow(dist * density, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = vec4(mix(col, fogColor, fogFactor), 1.0);
}