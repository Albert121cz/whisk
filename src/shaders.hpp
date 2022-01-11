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
    const int shaderType;
    bool initialized;

    Shader(GraphicsManager* parent, 
        const char* shaderFile, const int type);
    ~Shader() {if (initialized) glDeleteShader(ID);}
    std::string openFile(const char* filename);

private:
    GraphicsManager* parentManager;
};


#endif /* SHADERS_HPP_ */