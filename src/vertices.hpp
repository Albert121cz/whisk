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
    std::string textureName;

    Texture(GraphicsManager* parent, const unsigned char* imageData,
        int imageWidth, int imageHeight, std::string name);
    ~Texture();

    GLuint64 getHandle();

private:
    GraphicsManager* parentManager;
    GLuint ID;
    GLuint64 handle;
    GLenum texType;
};


class Object
{
public:
    bool show;
    std::string objectName;
    bool hasTex;
    std::shared_ptr<Texture> tex;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size;
    int renderMode;

    Object(GraphicsManager* parent, std::string name,
        std::shared_ptr<std::vector<GLfloat>> vert,
        std::shared_ptr<std::vector<GLfloat>> tex,
        std::shared_ptr<std::vector<GLfloat>> norm);
    ~Object();
    Object(const Object& oldObject);

    std::tuple<GLfloat, GLfloat, GLfloat> getColor();
    void setColor(GLfloat r, GLfloat g, GLfloat b);
    void draw();

private:
    GraphicsManager* parentManager;
    VertexBuffer* vertexBuffer;
    VertexArray* vertexArray;

    int vertexArrayStride;
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
    TextureManager(GraphicsManager* manager);

    void addTexture(const unsigned char* data, int width, int height,
        std::string name);
    void deleteTexture(int idx);
    std::shared_ptr<Texture> getTexPtr(int idx);
    std::vector<std::string> getAllTextureNames();

private:
    GraphicsManager* graphicsManager;
    std::vector<std::shared_ptr<Texture>> textures;
};


#endif /* VERTICES_HPP_ */
