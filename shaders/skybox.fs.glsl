#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform float nightFactor; // 0.0 = Tag, 1.0 = Nacht

void main()
{
    vec4 texColor = texture(skybox, TexCoords);

    vec3 dayColor = texColor.rgb;

    // Erzeugt Nacht-Look: Dunkler und höherer Kontrast durch Potenzierung
    vec3 nightColor = pow(dayColor, vec3(3.0));
    nightColor = nightColor * 0.1;

    // Mischt stufenlos zwischen Tag und Nacht
    vec3 finalColor = mix(dayColor, nightColor, nightFactor);

    FragColor = vec4(finalColor, 1.0);
}