#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float Brightness;

uniform vec3 birdColor;

void main()
{
    // Einfaches Lighting (von oben)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(Normal);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Ambient + Diffuse
    vec3 ambient = birdColor * 0.4;
    vec3 diffuse = birdColor * diff * 0.6;
    
    // Flügel-Animation Helligkeit einbeziehen
    vec3 color = (ambient + diffuse) * (0.8 + Brightness * 0.2);
    
    FragColor = vec4(color, 1.0);
}