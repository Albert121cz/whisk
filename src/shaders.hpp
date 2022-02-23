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
    Shader(GraphicsManager* parent, const char* shaderFile, const GLenum type);
    ~Shader();

    GLuint getID() const;
    bool getInitialized() const;

private:
    GLuint ID;
    GraphicsManager* parentManager;
    const GLenum shaderType;
    bool initialized;

    std::string openFile(const char* filename);
};

class ShaderManager
{
public:
    ShaderManager(GraphicsManager* parent);
    ~ShaderManager();

    void addShader(const char* file);
    void linkProgram();
    void useProgram();
    GLuint getID() const;

private:
    GLuint ID;
    GraphicsManager* parentManager;
    std::vector<std::unique_ptr<Shader>> vertexShaders;
    std::vector<std::unique_ptr<Shader>> fragmentShaders;
};


#endif /* SHADERS_HPP_ */
