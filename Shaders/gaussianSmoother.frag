#version 330 core

uniform sampler2D qt_texture;        // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    qt_fragColor  = 0.011343736 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2(-1,-1), 0);
    qt_fragColor += 0.083819505 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 0,-1), 0);
    qt_fragColor += 0.011343736 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 1,-1), 0);

    qt_fragColor += 0.083819505 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2(-1, 0), 0);
    qt_fragColor += 0.619347030 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 0, 0), 0);
    qt_fragColor += 0.083819505 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 1, 0), 0);

    qt_fragColor += 0.011343736 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2(-1, 1), 0);
    qt_fragColor += 0.083819505 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 0, 1), 0);
    qt_fragColor += 0.011343736 * texelFetch(qt_texture, ivec2(gl_FragCoord.xy) + ivec2( 1, 1), 0);
}
