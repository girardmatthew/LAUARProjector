#version 330 core

uniform sampler2D qt_depthTexture; // THIS TEXTURE HOLDS THE DFT COEFFICIENTS AS A 3-D TEXTURE

uniform sampler2D qt_minTexture;   // THIS TEXTURE HOLDS THE MINIMUM XYZP TEXTURE
uniform sampler2D qt_maxTexture;   // THIS TEXTURE HOLDS THE MAXIMUM XYZP TEXTURE
uniform sampler3D qt_lutTexture;   // THIS TEXTURE HOLDS THE LOOK UP TABLE XYZP TEXTURE
uniform     float qt_layers;       // THIS HOLDS THE NUMBER OF LAYERS IN THE LOOK UP TABLE
uniform      vec2 qt_depthLimits;  // THIS HOLDS THE RANGE LIMITS OF THE LOOK UP TABLE

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    int row = int(gl_FragCoord.y);
    int col = int(gl_FragCoord.x);

    // DERIVE THE MAGNITUDE TERM
    float magnitudeA = texelFetch(qt_depthTexture, ivec2(col,row), 0).x;
    float magnitudeB = texelFetch(qt_depthTexture, ivec2(col,row), 0).y;

    // DERIVE THE UNWRAPPED PHASE TERMS
    float phaseA = texelFetch(qt_depthTexture, ivec2(col,row), 0).z;

    // USE THE FOLLOWING COMMAND TO GET FOUR FLOATS FROM THE TEXTURE BUFFER
    float minPhase = texelFetch(qt_minTexture, ivec2(col,row), 0).a;
    float maxPhase = texelFetch(qt_maxTexture, ivec2(col,row), 0).a;

    // RESCALE THE PHASE TO BE IN THE SUPPLIED RANGE
    float phaseB = (phaseA - minPhase)/(maxPhase - minPhase);

    // EXTRACT THE XYZ COORDINATE FROM THE LOOKUP TABLE
    float lambda = qt_layers * phaseB - floor(qt_layers * phaseB);
    vec4 pixelA = texelFetch(qt_lutTexture, ivec3(col,row,int(floor(qt_layers * phaseB))), 0);
    vec4 pixelB = texelFetch(qt_lutTexture, ivec3(col,row,int( ceil(qt_layers * phaseB))), 0);

    // LINEARLY INTERPOLATE BETWEEN THE TWO LAYERS EXTRACTED FROM THE LOOK UP TABLE
    qt_fragColor = pixelA + lambda * (pixelB - pixelA);

    // DERIVE CUMMULATIVE FLAG TERM
    float rngFlag = float(phaseB > 0.0) * float(phaseB < 1.0);

    // APPLY SNR FLAG TO EITHER PRESERVE SAMPLE OR SET TO ALL NANS
    qt_fragColor.xyz = (qt_fragColor.xyz * rngFlag) / rngFlag;

    // SET THE OUTPUT SURFACE TEXTURE TO THE K=1 DFT COEFFICIENT'S MAGNITUDE
    qt_fragColor.a = (magnitudeA + magnitudeB);
}
