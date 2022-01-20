#include "vertices.hpp"


template <typename T>
void Buffer<T>::sendData(T* data, GLsizei size)
{
    glNamedBufferData(ID, size, data, GL_STATIC_DRAW);

    parentManager->oglErrorCheck(BUFFER_LOAD);
}


void VertexArray::enable()
{
    for (auto it = buffers.begin(); it != buffers.end(); it++)
        glBindBuffer((*it).first, (*it).second);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    parentManager->oglErrorCheck(VERTEX_ATTRIB);
    glEnableVertexAttribArray(0);
    parentManager->oglErrorCheck(ARRAY_ENABLE);

}


// explicit template instantiation
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
template class Buffer<GLfloat>;
template class Buffer<GLuint>;

