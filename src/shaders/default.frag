#version 460 core
out vec4 FragColor;

uniform vec4 outsideColor;

void main()
{
    FragColor = outsideColor;
}
