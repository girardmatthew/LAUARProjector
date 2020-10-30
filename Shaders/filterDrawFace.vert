#version 330 core

uniform sampler2D qt_mapping;      // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
in           vec4 qt_vertex;       // POINTS TO VERTICES PROVIDED BY USER ON CPU
out          vec2 qt_coordinate;   // OUTPUT COORDINATE TO FRAGMENT SHADER

void main(void)
{
    // PULL THE PROJECTOR COORDINATE FROM THE MAPPING TEXTURE
    gl_Position = texture(qt_mapping, qt_vertex.xy);

    // CONVERT THE TEXTURE COORDINATE TO A SCREEN COORDINATE
    gl_Position = vec4(2.0 * gl_Position.zw - 1.0, 0.0, 1.0);
    gl_Position.y = -gl_Position.y;

    // PASS THE TEXTURE COORDINATE OF TEMPLATE TO THE FRAGMENT SHADER
    qt_coordinate = qt_vertex.zw;
}
