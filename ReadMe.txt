# TerrainOpenGL
Stelle sicher, dass vcpkg installiert und in der Toolchain hinterlegt ist. Folgende Bibliotheken werden installiert:

* GLFW3
* GLM
* Assimp
* OpenGL
* Dear ImGui
* stb (stb_image)

## Bauanleitung

1. Terminal im Projektverzeichnis öffnen.
2. Build-Ordner erstellen:
   mkdir build
   cd build
3. CMake konfigurieren:
   cmake ..
4. Projekt kompilieren:
   cmake --build .

## Ausführung

Starten unter Linux/macOS:
./TerrainOpenGL

Starten unter Windows:
TerrainOpenGL.exe

## Projektstruktur

* src/: Enthält den gesamten Quellcode (.cpp und .h Dateien).
* libs/glad/: Enthält den OpenGL Loader.
* assets/: (Falls vorhanden) Hier sollten Texturen, Shader und Modelle abgelegt werden.