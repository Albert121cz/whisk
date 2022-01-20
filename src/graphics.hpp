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
class VertexArray;
class VertexBuffer;
class ElementBuffer;

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
    VertexArray* vertexArray;
    VertexBuffer* vertexBuffer;
    ElementBuffer* elementBuffer;
};


enum ErrorCause
{
    SHADER_CREATE,
    PROGRAM_LINK,
    PROGRAM_USE,
    BUFFER_LOAD,
    ARRAY_ENABLE,
    VERTEX_ATTRIB,
    DEL
};


#endif /* GRAPHICS_HPP_ */
