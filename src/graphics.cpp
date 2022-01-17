#include "main.hpp"
#include "graphics.hpp"

// https://github.com/VictorGordan/opengl-tutorials

GLfloat testVertices[] =
{
    -0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,
    0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f,
    0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f,
    -0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f,
    0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f,
    0.0f, -0.5f * float(sqrt(3)) / 3, 0.0f
};

GLuint testIndices[] =
{
    0, 3, 5,
    3, 2, 4,
    5, 4, 1,
};

GraphicsManager::GraphicsManager(Canvas* parent) : parentCanvas(parent)
{
    shaders = new ShaderManager(this);
    shaders->addShader("default.vert");
    shaders->addShader("default.frag");
    shaders->linkProgram();

    // GLuint vertexArrayObject, vertexBufferObject;
    // vertex array must be generated before the buffers
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glGenBuffers(1, &elementBufferObject);
    
    glBindVertexArray(vertexArrayObject);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(testVertices),
                            testVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(testIndices),
                                            testIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                        3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    oglErrorCheck(BUFFER_LOAD);

    // not necessary - just to be sure the buffer or array cannot be changed
    // vertex array must be unbound after the array buffer,
    // but before element buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


GraphicsManager::~GraphicsManager()
{
    delete shaders;
    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteBuffers(1, &elementBufferObject);
}


void GraphicsManager::render()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shaders->useProgram();
    glBindVertexArray(vertexArrayObject);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
    // parentCanvas->SwapBuffers();
}


void GraphicsManager::oglErrorCheck(int cause)
{
    #ifdef DEBUG
        std::ostringstream messageStream;
        std::string causeStr, errStr;
        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            messageStream.str("");

            switch (err)
            {
            case (GL_INVALID_ENUM):
                errStr = "Invalid enum";
                break;
            case (GL_INVALID_VALUE):
                errStr = "Invalid value";
                break;
            case (GL_INVALID_OPERATION):
                errStr = "Invalid operation";
                break;
            case (GL_STACK_OVERFLOW):
                errStr = "Stack overflow";
                break;
            case (GL_STACK_UNDERFLOW):
                errStr = "Stack underflow";
                break;
            case (GL_OUT_OF_MEMORY):
                errStr = "Out of memory";
                break;
            case (GL_TABLE_TOO_LARGE):
                errStr = "Table too large";
                break;
            default:
                errStr = "Unknown error";
                break;
            }

            switch (cause)
            {
            case (SHADER_CREATE):
                causeStr = "Shader creation";
                break;
            case (PROGRAM_LINK):
                causeStr = "Program linkage";
                break;
            case (PROGRAM_USE):
                causeStr = "Program usage";
                break;
            case (BUFFER_LOAD):
                causeStr = "Loading data into buffer";
                break;
            case (DEL):
                causeStr = "Deletion";
                break;
            }
            messageStream << "OpenGL ERROR occured!   Error code: " << errStr
                            << "   Last operation: " << causeStr;
            parentCanvas->log(messageStream.str());
        }
    #endif /* DEBUG */
}


void GraphicsManager::sendToLog(std::string message)
{
    #ifdef DEBUG
        parentCanvas->log(message);
    #endif /* DEBUG */
}
