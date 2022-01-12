#ifndef SHADERS_HPP_
#define SHADERS_HPP_

#include "main.hpp"
#include "graphics.hpp"

#include <string>
#include <fstream>

#ifdef DEBUG
    #include <sstream>
#endif /* DEBUG */  

class Shader
{
public:
    GLuint ID;

    Shader(GraphicsManager* parent, 
        const char* shaderFile, const GLenum type);
    ~Shader() {if (initialized) glDeleteShader(ID);}

private:
    const GLenum shaderType;
    GraphicsManager* parentManager;
    bool initialized;

    std::string openFile(const char* filename);
};


#endif /* SHADERS_HPP_ */