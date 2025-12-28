#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.TexCoords = aTexCoords;

    // Normal Matrix für korrekte Normalen-Rotation
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // Gram-Schmidt Orthogonalisierung (optional aber besser)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.Normal = N; // Für Slopes Berechnung brauchen wir die Geometrie-Normale
    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * worldPos;
}