#ifndef VERTICES_HPP_
#define VERTICES_HPP_

#include "main.hpp"

#include <memory>
#include <vector>
#include <GL/glew.h>

class GraphicsManager;

template <typename T>
class Buffer
{
public:
    Buffer(GraphicsManager* parent, const GLenum type) 
        : parentManager(parent), bufferType(type) {glCreateBuffers(1, &ID);}
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


class Texture
{
public:
    Texture(GraphicsManager* parent, const unsigned char* imageData,
        int imageWidth, int imageHeight, const GLenum type=GL_TEXTURE_2D);
    ~Texture() {glDeleteTextures(1, &ID);}

    void bind() {glBindTexture(texType, ID);}

private:
    GLuint ID;
    GraphicsManager* parentManager;
    const GLenum texType;
};


// TODO: this has to be reworked - every object will handle its texture
class TextureManager
{
public:
    TextureManager(GraphicsManager* parent) : parentManager(parent){};
    // ~TextureManager();

    void addTexture(const unsigned char* data, int width, int height)
        {textures.push_back(
            std::make_unique<Texture>(parentManager, data, width, height));}
    
    bool bindTex(unsigned int idx) {
        if (idx < textures.size()) 
            {textures[idx]->bind(); return true;}
        return false;}

private:
    GraphicsManager* parentManager;
    std::vector<std::unique_ptr<Texture>> textures;
};


#endif /* VERTICES_HPP_ */
