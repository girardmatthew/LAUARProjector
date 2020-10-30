#version 330 core

#define LOFREQUENCY     1.0
#define MEFREQUENCY     6.0
#define HIFREQUENCY    36.0

uniform sampler2D qt_depthTextureLo; // THIS TEXTURE HOLDS THE DFT COEFFICIENTS AS A 3-D TEXTURE
uniform sampler2D qt_depthTextureMe; // THIS TEXTURE HOLDS THE DFT COEFFICIENTS AS A 3-D TEXTURE
uniform sampler2D qt_depthTextureHi; // THIS TEXTURE HOLDS THE DFT COEFFICIENTS AS A 3-D TEXTURE

uniform     float qt_snrThreshold; // THIS HOLDS THE MAGNITUDE THRESHOLD
uniform     float qt_mtnThreshold; // THIS HOLDS THE MAGNITUDE THRESHOLD

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    int row = int(gl_FragCoord.y);
    int col = int(gl_FragCoord.x);

    // GET THE LOW FREQUENCY DFT COEFFICIENTS
    vec4 vertexLo = 16.0 * texelFetch(qt_depthTextureLo, ivec2(col,row), 0);
    vec4 vertexMe = 16.0 * texelFetch(qt_depthTextureMe, ivec2(col,row), 0);
    vec4 vertexHi = 16.0 * texelFetch(qt_depthTextureHi, ivec2(col,row), 0);

    // DERIVE THE WRAPPED PHASE TERMS
    vec2 phaseLo = 0.5 - vec2(atan(vertexLo.y, -vertexLo.x), atan(vertexLo.w, -vertexLo.z))/6.28318530717959;
    vec2 phaseMe = 0.5 - vec2(atan(vertexMe.y, -vertexMe.x), atan(vertexMe.w, -vertexMe.z))/6.28318530717959;
    vec2 phaseHi = 0.5 - vec2(atan(vertexHi.y, -vertexHi.x), atan(vertexHi.w, -vertexHi.z))/6.28318530717959;

    // DERIVE THE UNWRAPPED PHASE TERMS
    vec2 phase = (round(MEFREQUENCY/LOFREQUENCY*phaseLo - phaseMe) + phaseMe) / MEFREQUENCY * LOFREQUENCY;
         phase = (round(HIFREQUENCY/LOFREQUENCY*phase   - phaseHi) + phaseHi) / HIFREQUENCY * LOFREQUENCY;

    // DERIVE THE MAGNITUDE TERM
    float magnitudeA = 0.3334 * (length(vertexLo.xy) + length(vertexMe.xy) + length(vertexHi.xy));
    float magnitudeB = 0.3334 * (length(vertexLo.zw) + length(vertexMe.zw) + length(vertexHi.zw));

    // DERIVE CUMMULATIVE FLAG TERM
    float snrFlag = float(magnitudeA > qt_snrThreshold) * float(magnitudeB > qt_snrThreshold);
    float mgnFlag = float((magnitudeB/magnitudeA) < qt_mtnThreshold);
    float cumFlag = snrFlag * mgnFlag;

    // SET THE OUTPUT PIXEL
    qt_fragColor.xy = phase.xy * snrFlag;
    qt_fragColor.z = magnitudeA * snrFlag;
    qt_fragColor.w = magnitudeB * snrFlag;

    qt_fragColor.x = (qt_fragColor.x - (1.0 - 600.0/648.0)/2.0) * 648.0/600.0;
    qt_fragColor.y = (qt_fragColor.y - (1.0 - 800.0/960.0)/2.0) * 960.0/800.0;
}
