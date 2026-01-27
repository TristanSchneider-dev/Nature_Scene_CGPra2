#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

// Instancing support (locations 4-7 for mat4)
layout (location = 4) in vec4 instanceMatrix0;
layout (location = 5) in vec4 instanceMatrix1;
layout (location = 6) in vec4 instanceMatrix2;
layout (location = 7) in vec4 instanceMatrix3;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform bool useInstancing;

void main()
{
    mat4 finalModel = model;
    
    if (useInstancing) {
        // Reconstruct instance matrix
        finalModel = mat4(
            instanceMatrix0,
            instanceMatrix1,
            instanceMatrix2,
            instanceMatrix3
        );
    }
    
    gl_Position = lightSpaceMatrix * finalModel * vec4(aPos, 1.0);
}