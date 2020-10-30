#version 330 core

uniform sampler2D qt_texture;     // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
in           vec2 qt_vertex;      // ATTRIBUTE WHICH HOLDS THE ROW AND COLUMN COORDINATES FOR THE CURRENT PIXEL
out         float qt_z;           // STORES THE Z VALUE OF THE INCOMING VERTICES

void main(void)
{
    // USE THE FOLLOWING COMMAND TO GET FOUR FLOATS FROM THE TEXTURE BUFFER
    vec4 point = texelFetch(qt_texture, ivec2(qt_vertex), 0).rgba;

    // SAVE THE Z VALUE BEFORE APPLYING PROJECTION MATRIX
    qt_z = point.y;

    // PROJECT X,Y,Z COORDINATES TO THE GRAPHICS BUFFER
    gl_Position = vec4(2.0 * point.zwy - 1.0, 1.0);
}
