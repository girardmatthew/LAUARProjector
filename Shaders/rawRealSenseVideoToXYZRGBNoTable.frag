#version 330 core

in vec2 qt_coordinate;             // INPUT COORDINATE FROM VERTEX SHADER

uniform sampler2D qt_dptTexture;   // THIS TEXTURE HOLDS THE DFT COEFFICIENTS AS A 3-D TEXTURE
uniform      vec2 qt_dptLimits;    // THIS HOLDS THE RANGE LIMITS OF THE LOOK UP TABLE

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // CONVERT THE COLUMN COORDINATE TO PACKED COORDINATES
    int col = int(gl_FragCoord.x);
    int row = int(gl_FragCoord.y);

    // USE THE FOLLOWING COMMAND TO GET Z FROM THE DEPTH BUFFER
    float depthA = texelFetch(qt_dptTexture, ivec2(col,row), 0).r;

    // RESCALE THE PHASE TO BE IN THE SUPPLIED RANGE
    float depthB = (65535.0 * depthA - qt_dptLimits.x)/(qt_dptLimits.y - qt_dptLimits.x);

    // CLIP DEPTH B AND FORCE IT INSIDE THE LOOK UP TABLE RANGE
    float flag = 1.0;
    if (depthB < 0.05 || depthB > 0.95){
        flag = 0.0;
        depthB = 0.95;
    }

    // MERGE THE DEPTH VIDEO WITH THE PROJECTOR COORDINATES
    qt_fragColor = vec4(depthA, depthB, qt_coordinate);

    // APPLY A TEST TO THE PIXELS TO SEE IF THEY ARE IN THE VALID DEPTH RANGE
    // qt_fragColor.rg = ((qt_fragColor.rg * flag)/flag) * (flag/flag);
}
