#version 330 core

uniform      bool qt_portrait = false;   // THIS TELLS US IF WE WANT PORTRAIT MODE TO TRANSPOSE IMAGE
uniform sampler2D qt_texture;            // THIS TEXTURE HOLDS THE XYZ+TEXTURE COORDINATES
in           vec2 qt_coordinate;         // HOLDS THE TEXTURE COORDINATE FROM THE VERTEX SHADER

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    if (qt_portrait){
        qt_fragColor = texture(qt_texture, qt_coordinate.yx, 0).rrra;
    } else {
        qt_fragColor = texture(qt_texture, qt_coordinate.xy, 0).rrra;
    }
}
