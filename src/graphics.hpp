#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"

#include <wx/timer.h>
#include <GL/glew.h>
#include <string>
#include <chrono>

#ifdef DEBUG
    #include <sstream>
#endif /* DEBUG */

#define FPS 30

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


// https://wiki.wxwidgets.org/Making_a_render_loop
class RenderTimer : public wxTimer
{
public:
    RenderTimer(Canvas* parent);

    // function which is periodically triggered
    void Notify();

private:
    Canvas* parentCanvas;
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
