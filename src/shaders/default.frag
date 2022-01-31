#version 460 core
#extension ARB_bindless_texture : require

out vec4 finalColor;

in vec3 vertColor;
in vec2 vertTexCoord;

uniform int useTex;
uniform sampler2D texHandle;

void main()
{
    if (useTex == 1)
        finalColor = texture(texHandle, vertTexCoord);
    else 
        finalColor = vec4(vertColor, 0.0f);
}
