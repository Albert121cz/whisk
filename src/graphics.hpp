#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"

#include <GL/glew.h>
#include <string>

#ifdef DEBUG
    #include <sstream>
#endif /* DEBUG */

class Canvas;
class ShaderManager;

class GraphicsManager
{
public:
    GraphicsManager(Canvas* parent);
    ~GraphicsManager();
    void render();
    void oglErrorCheck(int cause);
    void sendToLog(std::string message);
private:
    Canvas* parentCanvas;
    ShaderManager* shaders;
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    GLuint elementBufferObject;
};


enum ErrorCause
{
    SHADER_CREATE,
    PROGRAM_LINK,
    PROGRAM_USE,
    BUFFER_LOAD,
    DEL
};


#endif /* GRAPHICS_HPP_ */
