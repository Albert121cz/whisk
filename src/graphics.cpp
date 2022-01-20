#include "graphics.hpp"

// https://github.com/VictorGordan/opengl-tutorials

GLfloat testVertices[] =
{
    0.0f, 1.0f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.0f, -0.0f, 0.5f,
};

GLuint testIndices[] =
{
    0, 1, 2,
    0, 1, 3,
    1, 2, 3,
    0, 2, 3
};


GraphicsManager::GraphicsManager(Canvas* parent) : parentCanvas(parent)
{
    shaders = new ShaderManager(this);
    shaders->addShader("default.vert");
    shaders->addShader("default.frag");
    shaders->linkProgram();

    // vertex array must be generated before the buffers
    vertexArray = new VertexArray(this);

    vertexBuffer = new VertexBuffer(this);
    elementBuffer = new ElementBuffer(this);

    vertexArray->bind();

    vertexBuffer->sendData(testVertices, sizeof(testVertices));
    elementBuffer->sendData(testIndices, sizeof(testIndices));

    vertexArray->link(vertexBuffer);
    vertexArray->link(elementBuffer);

    vertexArray->enable();
}


GraphicsManager::~GraphicsManager()
{
    delete shaders;
    delete vertexArray;
    delete vertexBuffer;
    delete elementBuffer;

    // vertexArray->deleteArray();
    // vertexBuffer->deleteBuffer();
    // elementBuffer->deleteBuffer();
}


void GraphicsManager::render()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    shaders->useProgram();
    vertexArray->bind();
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
            case (ARRAY_ENABLE):
                causeStr = "Enabling vertex array";
                break;
            case (VERTEX_ATTRIB):
                causeStr = "Making vertex attribute data array";
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
