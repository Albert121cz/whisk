#ifndef VERTICES_HPP_
#define VERTICES_HPP_

#include "main.hpp"

#include <memory>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class GraphicsManager;
class TextureManager;

template <typename T>
class Buffer
{
public:
    Buffer(GraphicsManager* parent, GLenum type);
    Buffer(const Buffer& old);
    ~Buffer();

    void sendData(T* data, GLsizei size);
    GLenum getType();
    GLuint getID();

protected:
    GLuint ID;
    GraphicsManager* parentManager;
    GLenum bufferType;
    T* dataStored;
    GLsizei dataStoredSize;
};

class VertexBuffer : public Buffer<GLfloat>
{
public:
    VertexBuffer(GraphicsManager* parent);
};


class ElementBuffer : public Buffer<GLuint>
{
public:
    ElementBuffer(GraphicsManager* parent);
};


class VertexArray
{
public:
    VertexArray(GraphicsManager* parent);
    ~VertexArray();

    void link(Buffer<GLfloat>* buffer);
    void link(Buffer<GLuint>* buffer);
    void enable();
    void bind();

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
    bool show;
    std::string objectName;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size;
    int renderMode;

    Object(GraphicsManager* parent, TextureManager* textures, std::string name,
        GLfloat* vert, size_t vertSize, GLuint* indices, size_t indSize);
    ~Object();
    Object(const Object& oldObject);

    void setColor(GLfloat r, GLfloat g, GLfloat b);
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
    GLfloat color[3];
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
