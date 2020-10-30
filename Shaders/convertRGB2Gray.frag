#version 330 core

uniform sampler2D qt_texture;            // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
in           vec2 qt_coordinate;         // HOLDS THE TEXTURE COORDINATE FROM THE VERTEX SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    float gray = dot(texture(qt_texture, qt_coordinate, 0).rrr, vec3(0.299, 0.587, 0.114));
    qt_fragColor = vec4(gray, gray, gray, 1.0);
}
