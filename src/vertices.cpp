#include "vertices.hpp"


VertexBuffer::VertexBuffer(GraphicsManager* parent)
    : parentManager(parent)
{
    dataStoredSize = 0;
    glCreateBuffers(1, &ID);
}


VertexBuffer::VertexBuffer(const VertexBuffer& old)
{
    parentManager = old.parentManager;
    dataStoredSize = old.dataStoredSize;
    dataStored = new GLfloat[dataStoredSize];

    for (GLsizei i = 0; i < dataStoredSize; i++)
        dataStored[i] = old.dataStored[i];

    glCreateBuffers(1, &ID);
    glNamedBufferData(ID, dataStoredSize * sizeof(GLfloat),
        dataStored, GL_STATIC_DRAW);
}


VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &ID);
    delete[] dataStored;
}


void VertexBuffer::sendData(GLfloat* data, GLsizei size)
{
    // data is stored inside the object for copying
    if (dataStoredSize != 0)
        delete dataStored;

    dataStoredSize = size;
    dataStored = new GLfloat[dataStoredSize];

    for (GLsizei i = 0; i < dataStoredSize; i++)
        dataStored[i] = data[i];

    glNamedBufferData(ID, size * sizeof(GLfloat), data, GL_STATIC_DRAW);
}


GLuint VertexBuffer::getID()
{
    return ID;
}


VertexArray::VertexArray()
{
    glGenVertexArrays(1, &ID);
}


VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &ID);
}


void VertexArray::enable()
{
    glBindVertexArray(ID);

    // bind linked buffers
    for (auto it = buffers.begin(); it != buffers.end(); it++)
        glBindBuffer((*it).first, (*it).second);

    // data structure inside vertex array
    //  pos  | color | tex | normal
    // X Y Z | R G B | X Y | X Y Z
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 
        (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 
        (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 
        (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 
        (GLvoid*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    // vertex array must be unbound first
    glBindVertexArray(0);

    // unbind linked buffers
    for (auto it = buffers.begin(); it != buffers.end(); it++)
        glBindBuffer((*it).first, 0);
}

void VertexArray::link(VertexBuffer* buffer)
{
    buffers.push_back(std::pair(GL_ARRAY_BUFFER, buffer->getID()));
}


void VertexArray::bind()
{
    glBindVertexArray(ID);
}


Texture::Texture(const unsigned char* imageData, int imageWidth,
    int imageHeight, std::string name)
    : textureName(name)
{
    glGenTextures(1, &ID);

    glBindTexture(GL_TEXTURE_2D, ID);
    
    // setting up down/upscaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);

    // setting up how the texture wraps around the object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // TODO: load and handle RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB,
        GL_UNSIGNED_BYTE, imageData);

    // OpenGL generates the remaining mipmap levels from the image
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}


void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, ID);
}


Object::Object(GraphicsManager* parent, std::string name, int lines,
    std::shared_ptr<std::vector<GLfloat>> vert,
    std::shared_ptr<std::vector<GLfloat>> texVert,
    std::shared_ptr<std::vector<GLfloat>> norm)
    :  objectName(name), parentManager(parent), lineCount(lines)
{
    show = true;
    tex = nullptr;
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    size = glm::vec3(1.0f, 1.0f, 1.0f);
    renderMode = FILL;
    color[0] = 1.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;

    vertexArrayStride = 11;

// combined array includes position of vertices (x, y, z), colors of vertices
// without texture (r, g, b), position of vertices in texture (x, y) and
// vertex normals for lighting (x, y, z)
    combinedLen = (vert->size() / 3) * vertexArrayStride;
    combinedData = new GLfloat[combinedLen];

    float texVal, normVal;

    for (size_t vertex = 0; vertex < vert->size() / 3; vertex++)
    {
        for (size_t coordIdx = 0; coordIdx < 3; coordIdx++)
            combinedData[vertex * vertexArrayStride + coordIdx] =
                vert->at(vertex * 3 + coordIdx);

        for (size_t clrIdx = 0; clrIdx < 3; clrIdx++)
            combinedData[vertex * vertexArrayStride + 3 + clrIdx] =
                color[clrIdx];

        for (size_t texIdx = 0; texIdx < 2; texIdx++)
        {
            if (texVert->size() <= vertex * 2 + texIdx)
                texVal = 0.0f;
            else
                texVal = texVert->at(vertex * 2 + texIdx);

            combinedData[vertex * vertexArrayStride + 6 + texIdx] = texVal;
        }

        for (size_t normIdx = 0; normIdx < 3; normIdx++)
        {
            if (norm->size() <= vertex * 3 + normIdx)
                normVal = 0.0f;
            else
                normVal = norm->at(vertex * 3 + normIdx);

            combinedData[vertex * vertexArrayStride + 8 + normIdx] = normVal;
        }
    }

    vertexBuffer = new VertexBuffer(parentManager);
    vertexBuffer->sendData(combinedData, combinedLen);

    vertexArray = new VertexArray();
    vertexArray->link(vertexBuffer);
    vertexArray->enable();
}


Object::~Object()
{
    delete[] combinedData;
    delete vertexArray;
    delete vertexBuffer;
}


Object::Object(const Object& old)
{
    show = old.show;
    objectName = old.objectName + " copy";
    position = old.position;
    rotation = old.rotation;
    size = old.size;
    renderMode = old.renderMode;

    for (int i = 0; i < 3; i++)
        color[i] = old.color[i];

    parentManager = old.parentManager;
    tex = old.tex;

    lineCount = old.lineCount;
    vertexArrayStride = old.vertexArrayStride;
    combinedLen = old.combinedLen;
    combinedData = new GLfloat[combinedLen];
    for (int i = 0; i < combinedLen; i++)
        combinedData[i] = old.combinedData[i];

    vertexBuffer = new VertexBuffer(*old.vertexBuffer);

    vertexArray = new VertexArray();
    vertexArray->link(vertexBuffer);
    vertexArray->enable();
}


std::tuple<GLfloat, GLfloat, GLfloat> Object::getColor()
{
    return std::make_tuple(color[0], color[1], color[2]);
}


void Object::setColor(GLfloat r, GLfloat g, GLfloat b) 
{
    color[0] = r;
    color[1] = g;
    color[2] = b;

    // color is on positions 3, 4 and 5
    for (int vertex = 0; vertex < combinedLen / vertexArrayStride; vertex++)
        for (int tone = 0; tone < 3; tone++)
            combinedData[vertex * vertexArrayStride + 3 + tone] = color[tone];
    
    vertexBuffer->sendData(combinedData, combinedLen);
}


void Object::draw()
{
    int useTexUniform = glGetUniformLocation(parentManager->getShadersID(),
        "useTex");

    GLint useTex;

    if (tex != nullptr)
    {
        useTex = 1;
        tex->bind();
    }
    else 
        useTex = 0;

    glUniform1i(useTexUniform, useTex);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotation.x),
        glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));
    model = glm::rotate(model, glm::radians(rotation.y),
        glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));
    model = glm::rotate(model, glm::radians(rotation.z),
        glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
    model = glm::translate(model, position);
    model = glm::scale(model, size);

    parentManager->setUniformMatrix(model, "model");

    GLenum oglRenderMode;
    switch(renderMode)
    {
        case FILL:
            oglRenderMode = GL_FILL;
        break;

        case LINE:
            oglRenderMode = GL_LINE;
        break;

        case POINT:
            oglRenderMode = GL_POINT;
        break;
    }
    glPolygonMode(GL_FRONT_AND_BACK, oglRenderMode);

    vertexArray->bind();
    glDrawArrays(GL_TRIANGLES, 0, combinedLen / 11 - lineCount / 3);
    glDrawArrays(GL_LINES, combinedLen / 11 - lineCount / 3, combinedLen / 11);
    glBindTexture(GL_TEXTURE_2D, 0);
}
