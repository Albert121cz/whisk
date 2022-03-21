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

class VertexBuffer
{
public:
    VertexBuffer(GraphicsManager* parent);
    VertexBuffer(const VertexBuffer& old);
    ~VertexBuffer();

    void sendData(GLfloat* data, GLsizei size);
    GLuint getID();

protected:
    GLuint ID;
    GraphicsManager* parentManager;
    GLfloat* dataStored;
    GLsizei dataStoredSize;
};


class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void link(VertexBuffer* buffer);
    void enable();
    void bind();

private:
    GLuint ID;
    std::vector<std::pair<GLenum, GLuint>> buffers;
};


class Texture
{
public:
    std::string textureName;

    Texture(const unsigned char* imageData, int imageWidth, int imageHeight,
        std::string name);
    ~Texture();

    void bind();

private:
    GLuint ID;
};


class Object
{
public:
    bool show;
    std::string objectName;
    std::shared_ptr<Texture> tex;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size;
    int renderMode;

    Object(GraphicsManager* parent, std::string name, int lineCount,
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

    int lineCount;
    int vertexArrayStride;
    int combinedLen;
    GLfloat* combinedData;

    GLfloat color[3];

    enum RenderMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2
    };
};


#endif /* VERTICES_HPP_ */
