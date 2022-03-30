#ifndef SHADERS_HPP_
#define SHADERS_HPP_

#include "main.hpp"

#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <memory>
#include <GL/glew.h>

#ifdef DEBUG
    #include <sstream>
#endif /* DEBUG */

class GraphicsManager;

class Shader
{
public:
    Shader(const char* shaderFile, const GLenum type);
    ~Shader();

    GLuint getID() const;
    bool getInitialized() const;

private:
    GLuint ID;
    const GLenum shaderType;
    bool initialized;

    std::string openFile(const char* filename);
};

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    void addShader(const char* file);
    bool linkProgram();
    void useProgram();
    GLuint getID() const;

private:
    GLuint ID;
    std::vector<std::unique_ptr<Shader>> vertexShaders;
    std::vector<std::unique_ptr<Shader>> fragmentShaders;
};


#endif /* SHADERS_HPP_ */
