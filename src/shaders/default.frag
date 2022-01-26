#version 460 core
out vec4 FragColor;

in vec3 vertColor;
in vec2 vertTexCoord;

uniform int useTex;
uniform sampler2D texImage;

void main()
{
    if (useTex == 0)
        FragColor = vec4(vertColor, 0.0f);
    else 
        FragColor = texture(texImage, vertTexCoord);
}
