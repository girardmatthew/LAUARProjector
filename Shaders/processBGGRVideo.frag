#version 330 core

uniform sampler2D qt_texture;            // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES
uniform float     qt_scaleFactor = 1.0f; // SCALE VIDEO IF ITS NOT ENTIRELY 8 OR 16 BITS PER PIXEL

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    ivec2 coordA = ivec2(gl_FragCoord);
    ivec2 coordB = coordA%2;

    if (coordB.x == 0 && coordB.y == 0){
        float red = texelFetch(qt_texture, coordA + ivec2(1,1), 0).r;
        float grn = texelFetch(qt_texture, coordA + ivec2(1,0), 0).r + texelFetch(qt_texture, coordA + ivec2(0,1), 0).r;
        float blu = texelFetch(qt_texture, coordA + ivec2(0,0), 0).r;
        qt_fragColor = vec4(red, grn/2.0, blu, 1.0);
    } else if (coordB.x == 1 && coordB.y == 0){
        float red = texelFetch(qt_texture, coordA + ivec2(0,1), 0).r;
        float grn = texelFetch(qt_texture, coordA + ivec2(0,0), 0).r;
        float blu = texelFetch(qt_texture, coordA - ivec2(1,0), 0).r;
        qt_fragColor = vec4(red, grn, blu, 1.0);
    } else if (coordB.x == 0 && coordB.y == 1){
        float red = texelFetch(qt_texture, coordA + ivec2(1,0), 0).r;
        float grn = texelFetch(qt_texture, coordA + ivec2(0,0), 0).r;
        float blu = texelFetch(qt_texture, coordA - ivec2(0,1), 0).r;
        qt_fragColor = vec4(red, grn, blu, 1.0);
    } else {
        float red = texelFetch(qt_texture, coordA + ivec2(0,0), 0).r;
        float grn = texelFetch(qt_texture, coordA - ivec2(1,0), 0).r + texelFetch(qt_texture, coordA - ivec2(0,1), 0).r;
        float blu = texelFetch(qt_texture, coordA - ivec2(1,1), 0).r;
        qt_fragColor = vec4(red, grn/2.0, blu, 1.0);
    }
    qt_fragColor.rgb = qt_fragColor.rgb * qt_scaleFactor;
}
