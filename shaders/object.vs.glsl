#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
// NEU: Matrix für Instancing (belegt Location 4, 5, 6, 7)
layout (location = 4) in mat4 aInstanceMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// NEU: Schalter
uniform bool useInstancing;

void main()
{
    // WENN Instancing an ist, nimm die Matrix aus dem Puffer, SONST die normale Uniform
    mat4 currentModel = useInstancing ? aInstanceMatrix : model;

    WorldPos = vec3(currentModel * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(currentModel)));
    vec3 N = normalize(normalMatrix * aNormal);
    Normal = N;

    vec3 T = normalize(normalMatrix * aTangent);
    // Gram-Schmidt / Parallel-Fix (Dein Fix von vorhin)
    if (abs(dot(N, T)) > 0.99) {
         vec3 helpAxis = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
         T = normalize(cross(N, helpAxis));
    }
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}