#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"
#include <GL/glew.h>
#include <iostream>

class Canvas;

class GraphicsManager
{
public:
    GraphicsManager(Canvas*);
    ~GraphicsManager();
    void render();
private:
    Canvas* parentCanvas;
    GLuint vertexArrayObject;
    GLuint vertexBufferObject;
    GLuint shaderProgram;

    void errorCheck(int cause);
};

#endif /* GRAPHICS_HPP_ */