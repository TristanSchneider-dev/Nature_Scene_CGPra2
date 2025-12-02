#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    WorldPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    // Normalen Matrix
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    Normal = N;

    // Tangente transformieren
    vec3 T = normalize(normalMatrix * aTangent);

    // [FIX] Sicherheits-Check: Sind N und T parallel? (Das verursacht die schwarzen Flecken)
    // Wenn das Skalarprodukt nahe 1 oder -1 ist, zeigen sie in die gleiche Richtung.
    if (abs(dot(N, T)) > 0.99) {
        // Wir erfinden eine neue Tangente, die NICHT parallel zu N ist.
        // Wenn N fast senkrecht nach oben zeigt, nimm X-Achse, sonst Y-Achse.
        vec3 helpAxis = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
        T = normalize(cross(N, helpAxis));
    }

    // Gram-Schmidt Re-Orthogonalisierung (macht T perfekt rechtwinklig zu N)
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    // TBN Matrix erstellen
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}