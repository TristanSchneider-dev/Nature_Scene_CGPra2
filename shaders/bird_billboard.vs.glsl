#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out float AnimFrame;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraPos;
uniform vec3 birdPos;
uniform float birdScale;
uniform float wingPhase;
uniform float wingSpeed;
uniform float time;
uniform int frameCount;

void main()
{
    // Billboard: Quad schaut immer zur Kamera
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    // Skaliere Quad
    vec3 worldPos = birdPos 
                  + cameraRight * aPos.x * birdScale 
                  + cameraUp * aPos.y * birdScale;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    // UV Koordinaten
    TexCoords = aTexCoords;
    
    // Berechne Animation Frame (Flügelschlag)
    float animSpeed = wingSpeed * 8.0;
    AnimFrame = mod(time * animSpeed + wingPhase, float(frameCount));
}