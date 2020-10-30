#version 410 core

uniform    mat3x3 qt_transform;    // THIS MATRIX CONVERTS FROM ROW/COLUMN TO WORLD XY
in           vec2 qt_coordinate;   // HOLDS THE TEXTURE COORDINATE FROM THE VERTEX SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // TRANSFORM THE PROJECTOR SCREEN COORDINATE INTO A CAMERA
    // TEXTURE COORDINATE USING THE SUPPLIED 3X3 TRANSFORM MATRIX
    qt_fragColor.xyz = qt_transform * vec3(qt_coordinate.xy, 1.0);

    // SET THE Z AND W TO THEIR DEFAULT COORDINATES
    qt_fragColor.z = 0.0;
    qt_fragColor.w = 1.0;
}
