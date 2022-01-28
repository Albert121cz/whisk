#include "vertices.hpp"


template <typename T>
void Buffer<T>::sendData(T* data, GLsizei size)
{
    glNamedBufferData(ID, size, data, GL_STATIC_DRAW);
}


void VertexArray::enable()
{
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
}


// explicit template instantiation
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
template class Buffer<GLfloat>;
template class Buffer<GLuint>;
