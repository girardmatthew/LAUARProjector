#version 330 core

in float qt_z;   // STORES THE Z VALUE OF THE INCOMING VERTICES

layout(location = 0, index = 0) out vec4 qt_fragColor;

void main()
{
    // GET THE PIXEL COORDINATE OF THE CURRENT FRAGMENT
    float gray = 30.0 * qt_z;
    gray = gray - floor(gray);
    qt_fragColor = vec4(gray, gray, gray, 1.0); //qt_z, qt_z, 1.0);
}
