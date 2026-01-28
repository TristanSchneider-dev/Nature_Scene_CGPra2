#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float AnimFrame;

uniform sampler2D birdTexture;
uniform int frameCount;

void main()
{
    // Sprite Sheet UV Berechnung
    // Annahme: Frames sind horizontal angeordnet
    int currentFrame = int(AnimFrame);
    float frameWidth = 1.0 / float(frameCount);
    
    vec2 uv = TexCoords;
    uv.x = (uv.x * frameWidth) + (float(currentFrame) * frameWidth);
    
    vec4 texColor = texture(birdTexture, uv);
    
    // Alpha cutoff für transparente Bereiche
    if (texColor.a < 0.1)
        discard;
    
    // Einfache Schattierung
    vec3 color = texColor.rgb * 0.8; // Leicht abdunkeln
    
    FragColor = vec4(color, texColor.a);
}