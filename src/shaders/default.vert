#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertColor;
out vec2 vertTexCoord;
out vec3 vertNormal;
out vec3 vertPos;

void main()
{
    gl_Position = projection * view * model * vec4(inPos, 1.0);
    vertColor = inColor;
    vertTexCoord = inTexCoord;
    vertNormal = mat3(transpose(inverse(model))) * inNormal;
    vertPos = vec3(model * vec4(inPos, 1.0f));
}
