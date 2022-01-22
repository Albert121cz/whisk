#ifndef VERTICES_HPP_
#define VERTICES_HPP_

#include "main.hpp"

#include <vector>
#include <GL/glew.h>

class GraphicsManager;

template <typename T>
class Buffer
{
public:
    Buffer(GraphicsManager* parent, const GLenum type) 
        : parentManager(parent), bufferType(type) {glGenBuffers(1, &ID);}
    ~Buffer() {glDeleteBuffers(1, &ID);}

    void sendData(T* data, GLsizei size);
    GLenum getType() {return bufferType;}
    GLuint getID() {return ID;}

protected:
    GLuint ID;
    GraphicsManager* parentManager;
    const GLenum bufferType;
};

class VertexBuffer : public Buffer<GLfloat>
{
public:
    VertexBuffer(GraphicsManager* parent)
        : Buffer<GLfloat>(parent, GL_ARRAY_BUFFER){};
};


class ElementBuffer : public Buffer<GLuint>
{
public:
    ElementBuffer(GraphicsManager* parent)
        : Buffer<GLuint>(parent, GL_ELEMENT_ARRAY_BUFFER){};
};


class VertexArray
{
public:
    VertexArray(GraphicsManager* parent) : parentManager(parent) 
        {glGenVertexArrays(1, &ID);}
    ~VertexArray() {glDeleteVertexArrays(1, &ID);}

    void link(Buffer<GLfloat>* buffer)
        {buffers.push_back(std::pair(buffer->getType(), buffer->getID()));}
    void link(Buffer<GLuint>* buffer)
        {buffers.push_back(std::pair(buffer->getType(), buffer->getID()));}
    void enable();
    void bind() {glBindVertexArray(ID);}

private:
    GLuint ID;
    GraphicsManager* parentManager;
    std::vector<std::pair<GLenum, GLuint>> buffers;

};

#endif /* VERTICES_HPP_ */
