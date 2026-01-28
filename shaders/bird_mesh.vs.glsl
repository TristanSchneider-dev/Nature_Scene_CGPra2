#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float aIsWing; // 0.0 = Körper, 1.0 = Flügel

out vec3 FragPos;
out vec3 Normal;
out float Brightness;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float wingPhase;
uniform float wingSpeed;

void main()
{
    vec3 pos = aPos;
    vec3 normal = aNormal;
    
    // Flügel-Animation nur für Wing-Vertices
    if (aIsWing > 0.5) {
        // Sinus-Welle für Flügelschlag
        float wingAngle = sin(time * wingSpeed * 8.0 + wingPhase) * 0.6; // 0.6 radians = ~34 Grad
        
        // Rotiere Flügel um Z-Achse (auf/ab Bewegung)
        // X-Position entscheidet ob linker oder rechter Flügel
        float side = sign(aPos.x); // -1 für links, +1 für rechts
        
        // Y-Offset für Flügelschlag
        float wingLift = sin(wingAngle) * abs(aPos.x) * 0.8;
        pos.y += wingLift;
        
        // Leichte Z-Bewegung für realistischeren Flug
        pos.z -= cos(wingAngle) * abs(aPos.x) * 0.1;
        
        // Normale auch anpassen für besseres Lighting
        normal.y += wingLift * 0.5;
        normal = normalize(normal);
    }
    
    // Transformiere Position
    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    
    // Einfache Helligkeit basierend auf Flügelposition (sieht cool aus!)
    Brightness = 0.5 + 0.5 * sin(time * wingSpeed * 8.0 + wingPhase);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}