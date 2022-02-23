#ifndef VERTICES_HPP_
#define VERTICES_HPP_

#include "main.hpp"

#include <memory>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

#define DEFAULT_COLOR 1.0f, 0.0f, 0.0f

class GraphicsManager;
class TextureManager;

template <typename T>
class Buffer
{
public:
    Buffer(GraphicsManager* parent, GLenum type) 
        : parentManager(parent), bufferType(type) {glCreateBuffers(1, &ID);}
    Buffer(const Buffer& old);
    ~Buffer() {glDeleteBuffers(1, &ID); delete[] dataStored;}

    void sendData(T* data, GLsizei size);
    GLenum getType() {return bufferType;}
    GLuint getID() {return ID;}

protected:
    GLuint ID;
    GraphicsManager* parentManager;
    GLenum bufferType;
    T* dataStored;
    GLsizei dataStoredSize = 0;
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

    GLuint64 getHandle() {return handle;}

private:
    GLuint ID;
    GLuint64 handle;
    GraphicsManager* parentManager;
    const GLenum texType;
};


class Object
{
public:
    bool show = true;
    std::string objectName;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
    int renderMode = 0;

    Object(GraphicsManager* parent, TextureManager* textures, std::string name,
        GLfloat* vert, size_t vertSize, GLuint* indices, size_t indSize);
    ~Object();
    Object(const Object& oldObject);

    void setColor(GLfloat r, GLfloat g, GLfloat b) 
        {color[0] = r; color[1] = g; color[2] = b;}
    // TODO: link texture with object - needs texture menu (wxListBox)
    void setTexture(unsigned int idx);
    void draw();

private:
    GraphicsManager* parentManager;
    TextureManager* texManager;
    VertexBuffer* vertexBuffer;
    ElementBuffer* elementBuffer;
    VertexArray* vertexArray;
    std::shared_ptr<Texture> tex;

    int indicesLen;
    int combinedLen;
    GLfloat* combinedData;

    GLuint64 texHandle;
    GLfloat color[3] = {DEFAULT_COLOR};
    glm::mat4 model;

    enum RenderMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2
    };
};


class TextureManager
{
public:
    TextureManager(GraphicsManager* parent) : parentManager(parent){};
    // ~TextureManager();

    void addTexture(const unsigned char* data, int width, int height)
        {textures.push_back(
            std::make_shared<Texture>(parentManager, data, width, height));}
    
    std::shared_ptr<Texture> getTexPtr(unsigned int idx)
        {return (idx < textures.size() ? textures[idx] : nullptr);}

private:
    GraphicsManager* parentManager;
    std::vector<std::shared_ptr<Texture>> textures;
};


#endif /* VERTICES_HPP_ */
