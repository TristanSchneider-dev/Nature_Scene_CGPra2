#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    // Wir rendern direkt im Clip Space (-1 bis 1), keine Matrix nötig
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}