#version 330 core

uniform sampler2D qt_texture;        // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES
uniform vec4      qt_scaleFactor;    // SCALE VIDEO IF ITS NOT ENTIRELY 8 OR 16 BITS PER PIXEL

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    qt_fragColor = qt_scaleFactor * texelFetch(qt_texture, ivec2(gl_FragCoord.xy), 0).rrrr;
}
