#version 330 core

uniform sampler2D qt_texture;        // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES
uniform     float qt_threshold;      // HOLDS THE MAXIMUM DELTA BETWEEN Z-COORDINATES
uniform     float qt_width;

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    ivec2 coord = ivec2(gl_FragCoord.xy);

    if (coord.x % 2 == 0){
        coord.x /= 2;
        vec4 pixelA = texelFetch(qt_texture, coord + ivec2(0,0), 0);
        vec4 pixelB = texelFetch(qt_texture, coord + ivec2(1,0), 0);
        vec4 pixelC = texelFetch(qt_texture, coord + ivec2(0,1), 0);

        float flagAB = float(abs(pixelA.z - pixelB.z) < qt_threshold);
        float flagAC = float(abs(pixelA.z - pixelC.z) < qt_threshold);
        float flagBC = float(abs(pixelB.z - pixelC.z) < qt_threshold);

        float flag = flagAB * flagAC * flagBC;
        float anchor = float(coord.x) + float(coord.y) * qt_width;
        qt_fragColor = vec4(anchor, anchor + 1.0 + qt_width, anchor + 1.0, flag);
        qt_fragColor.rgb = (qt_fragColor.rgb * flag) / flag * (flag/flag);
    } else {
        coord.x /= 2;
        vec4 pixelA = texelFetch(qt_texture, coord + ivec2(0,0), 0);
        vec4 pixelC = texelFetch(qt_texture, coord + ivec2(0,1), 0);
        vec4 pixelD = texelFetch(qt_texture, coord + ivec2(1,1), 0);

        float flagAC = float(abs(pixelA.z - pixelC.z) < qt_threshold);
        float flagAD = float(abs(pixelA.z - pixelD.z) < qt_threshold);
        float flagCD = float(abs(pixelC.z - pixelD.z) < qt_threshold);

        float flag = flagAD * flagCD * flagAC;
        float anchor = float(coord.x) + float(coord.y) * qt_width;
        qt_fragColor = vec4(anchor, anchor + 1.0 + qt_width, anchor + qt_width, flag);
        qt_fragColor.rgb = (qt_fragColor.rgb * flag) / flag * (flag/flag);
    }
}
