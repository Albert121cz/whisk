#include "vertices.hpp"


template <typename T>
Buffer<T>::Buffer(const Buffer& old)
{
    
    parentManager = old.parentManager;
    bufferType = old.bufferType;
    dataStoredSize = old.dataStoredSize;
    dataStored = new T[dataStoredSize];

    for (GLsizei i = 0; i < dataStoredSize; i++)
        dataStored[i] = old.dataStored[i];

    glCreateBuffers(1, &ID);
    glNamedBufferData(ID, dataStoredSize*sizeof(T), dataStored, GL_STATIC_DRAW);
}


template <typename T>
void Buffer<T>::sendData(T* data, GLsizei size)
{
    // data is stored inside the object for copying
    if (dataStoredSize != 0)
        delete dataStored;

    dataStoredSize = size / sizeof(T);
    dataStored = new T[dataStoredSize];

    for (GLsizei i = 0; i < dataStoredSize; i++)
        dataStored[i] = data[i];

    glNamedBufferData(ID, size, data, GL_STATIC_DRAW);
}


void VertexArray::enable()
{
    glBindVertexArray(ID);

    // bind linked buffers
    for (auto it = buffers.begin(); it != buffers.end(); it++)
        glBindBuffer((*it).first, (*it).second);

    // X Y Z | R G B | S T
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 
        (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 
        (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 
        (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // vertex array must be unbound first
    glBindVertexArray(0);

    // unbind linked buffers
    for (auto it = buffers.begin(); it != buffers.end(); it++)
        glBindBuffer((*it).first, 0);
}


Texture::Texture(GraphicsManager* parent, const unsigned char* imageData, 
    int imageWidth, int imageHeight, const GLenum type)
    : parentManager(parent), texType(type)
{
    glGenTextures(1, &ID);

    glBindTexture(texType, ID);
    
    // setting up down/upscaling
    glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // setting up how the texture wraps around the object
    glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // TODO: load and handle RGBA
    glTexImage2D(texType, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB,
        GL_UNSIGNED_BYTE, imageData);

    // OpenGL generates the remaining mipmap levels from the image
    glGenerateMipmap(texType);

    glBindTexture(texType, 0);

    handle = glGetTextureHandleARB(ID);
    glMakeTextureHandleResidentARB(handle);
}


Object::Object(GraphicsManager* parent, TextureManager* textures,
    std::string name, GLfloat* vert, size_t vertSize, GLuint* indices,
    size_t indSize)
    :  objectName(name), parentManager(parent), texManager(textures),
    indicesLen(indSize/sizeof(GLuint))
{
// combined array includes position of vertices (x, y, z), colors of vertices
// without texture (r, g, b), position of vertices in texture (s, t)
    int verticesLen = vertSize/sizeof(GLfloat);
    combinedLen = (verticesLen / 3) * 8;
    combinedData = new GLfloat[combinedLen];
    
    for (int vertex = 0; vertex < verticesLen / 3; vertex++)
    {
        for (size_t coord = 0; coord < 3; coord++)
            combinedData[vertex * 8 + coord] = vert[vertex * 3 + coord];

        for (size_t clr = 0; clr < 3; clr++)
            combinedData[vertex * 8 + 3 + clr] = color[clr];
    }

    vertexBuffer = new VertexBuffer(parentManager);
    vertexBuffer->sendData(combinedData, combinedLen*sizeof(GLfloat));

    elementBuffer = new ElementBuffer(parentManager);
    elementBuffer->sendData(indices, indicesLen*sizeof(GLuint));

    vertexArray = new VertexArray(parentManager);
    vertexArray->link(vertexBuffer);
    vertexArray->link(elementBuffer);
    vertexArray->enable();
}


Object::~Object()
{
    delete[] combinedData;   
    delete vertexArray;
    delete vertexBuffer;
    delete elementBuffer;
}


Object::Object(const Object& old)
{
    show = old.show;
    objectName = old.objectName + " copy";

    parentManager = old.parentManager;
    texManager = old.texManager;
    tex = old.tex;

    indicesLen = old.indicesLen;
    combinedLen = old.combinedLen;
    combinedData = new GLfloat[combinedLen];
    for (int i = 0; i < combinedLen; i++)
        combinedData[i] = old.combinedData[i];

    for (int i = 0; i < 3; i++)
        color[i] = old.color[i];
    position = old.position;
    rotation = old.rotation;
    size = old.size;
    renderMode = old.renderMode;

    vertexBuffer = new VertexBuffer(*old.vertexBuffer);
    elementBuffer = new ElementBuffer(*old.elementBuffer);

    vertexArray = new VertexArray(parentManager);
    vertexArray->link(vertexBuffer);
    vertexArray->link(elementBuffer);
    vertexArray->enable();
}


void Object::setTexture(unsigned int idx)
{
    tex = texManager->getTexPtr(idx);
}


void Object::draw()
{
    int useTexUniform = glGetUniformLocation(parentManager->getShadersID(),
        "useTex");
    int samplerUniform = glGetUniformLocation(parentManager->getShadersID(),
        "texHandle");
    GLint hasTex;
    if (tex != nullptr)
    {
        hasTex = 1;
        GLuint64 handle = tex->getHandle();
        glUniformHandleui64ARB(samplerUniform, handle);
    }
    else hasTex = 0;
    glUniform1i(useTexUniform, hasTex);
    
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
    glDrawElements(GL_TRIANGLES, indicesLen*sizeof(GLuint), GL_UNSIGNED_INT, 0);
}


// explicit template instantiation
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
template class Buffer<GLfloat>;
template class Buffer<GLuint>;
