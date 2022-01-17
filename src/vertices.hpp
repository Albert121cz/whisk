#ifndef VERTICES_HPP_
#define VERTICES_HPP_

#include "main.hpp"

/*
template <typename T>
class Buffer
{
public:
    Buffer(T[] data, const GLenum type);

    void deleteBuffer();

private:
    GLuint ID;
    const GLenum bufferType;
};


template <typename T>
class VertexArray
{
public:
    VertexArray(GraphicsManager* parent) : parentManager(parent) 
                {glGenVertexArrays(1, &ID);}
    ~VertexArray() {glDeleteVertexArrays(1, &ID);}

    void addBuffer(Buffer<T>* buffer);

private:
    GLuint ID;
    GraphicsManager* parentManager;
    std::vector<std::pair<GLenum, GLuint>> buffers;

};

*/
#endif /* VERTICES_HPP_ */
