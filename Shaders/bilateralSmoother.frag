#version 330 core

uniform sampler2D qt_texture;   // THIS TEXTURE HOLDS THE RGB TEXTURE COORDINATES
uniform       int qt_radius;
uniform     float qt_sigmaX;
uniform     float qt_sigmaZ;

layout(location = 0, index = 0) out vec4 qt_fragColor;

float weight(vec4 point)
{
    float wghtX = -dot(point.xy,point.xy)/(2.0*qt_sigmaX);
    float wghtZ = -(point.z*point.z)/(2.0*qt_sigmaZ);

    return(exp(wghtX+wghtZ));
}

void main()
{
    // GET THE FRAGMENT PIXEL COORDINATE
    ivec2 coord = ivec2(gl_FragCoord.xy);
    qt_fragColor = texelFetch(qt_texture, coord, 0);

    // EXTRACT THE REGION OF INTEREST AROUND THE CURRENT INPUT PIXEL
    float cumSum = 0.0;
    float cumWgt = 0.0;
    for (int r = -qt_radius; r <= qt_radius; r++){
        for (int c = -qt_radius; c <= qt_radius; c++){
            vec4 pixel = texelFetch(qt_texture, coord + ivec2(c,r), 0) - qt_fragColor;
            if (isnan(pixel.x) == false){
                float w = weight(pixel);
                cumWgt += w;
                cumSum += w * pixel.z;
            }
        }
    }
    qt_fragColor.z += cumSum/cumWgt;

    return;
}

