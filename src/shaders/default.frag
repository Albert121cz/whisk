#version 460 core
#extension GL_ARB_bindless_texture : require

out vec4 finalColor;

in vec3 vertColor;
in vec2 vertTexCoord;
in vec3 vertNormal;
in vec3 vertPos;

uniform int useTex;
uniform sampler2D texHandle;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{
    if (length(vertNormal) == 0)
    {
        finalColor = vec4(vertColor, 1.0f);
        return;
    }
    
    float ambientLightStrength = 0.1f;
    vec3 ambientLight = ambientLightStrength * lightColor;

    // if cos < 0 (angle>90°), keep at 0
    float diffuseLightStrength = 0.85f;
    float diffuse = max(
        dot(normalize(vertNormal), normalize(lightPos - vertPos)), 0.0f);
    vec3 diffuseLight = diffuse * diffuseLightStrength * lightColor;

    if (useTex == 1)
    {
        vec4 lightVec = vec4((ambientLight + diffuseLight), 1.0f);
        finalColor = lightVec * texture(texHandle, vertTexCoord);
        return;
    }

    finalColor = vec4((ambientLight + diffuseLight) * vertColor, 1.0f);
}
