#include "graphics.hpp"

// https://github.com/VictorGordan/opengl-tutorials
// https://learnopengl.com/

// TODO: split into 3 components, make Object class
GLfloat testVertices[] =
{
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f
};

GLuint testIndices[] =
{
    0, 1, 2,
    0, 2, 3,
};


GraphicsManager::GraphicsManager(Canvas* parent) : parentCanvas(parent)
{
    shaders = new ShaderManager(this);
    shaders->addShader("default.vert");
    shaders->addShader("default.frag");
    shaders->linkProgram();

    textures = new TextureManager(this);

    vertexBuffer = new VertexBuffer(this);
    vertexBuffer->sendData(testVertices, sizeof(testVertices));

    elementBuffer = new ElementBuffer(this);
    elementBuffer->sendData(testIndices, sizeof(testIndices));

    vertexArray = new VertexArray(this);
    vertexArray->bind();
    vertexArray->link(vertexBuffer);
    vertexArray->link(elementBuffer);
    vertexArray->enable();

    camera = new Camera();
}


GraphicsManager::~GraphicsManager()
{
    delete shaders;
    delete textures;
    delete vertexArray;
    delete vertexBuffer;
    delete elementBuffer;
    delete camera;
}


void GraphicsManager::render()
{
    glClearColor(0.135f, 0.135f, 0.135f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shaders->useProgram();

    int texUniform = glGetUniformLocation(shaders->getID(), "useTex");
    if (textures->bindTex(0))
        glUniform1i(texUniform, 1);
    else
        glUniform1i(texUniform, 0);

    camera->move(parentCanvas->getCameraMouseInfo());
    
    int modelMat = glGetUniformLocation(shaders->getID(), "model");
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(45.0f), glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
    model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.25f, 1.25f, 1.25f));
    glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model));

    setUniformMatrix(camera->viewMatrix(), "view");

    setUniformMatrix(camera->projectionMatrix(parentCanvas->viewportAspectRatio()), "projection");


    vertexArray->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
            case (TEX_LOAD):
                causeStr = "Loading texture";
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


void GraphicsManager::setUniformMatrix(glm::mat4 mat, const char* name)
{
    int location = glGetUniformLocation(shaders->getID(), name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}


RenderTimer::RenderTimer(Canvas* parent) : parentCanvas(parent)
{
    StartOnce(0);
}


void RenderTimer::Notify()
{
    parentCanvas->flip();
    StartOnce();
}


glm::mat4 Camera::viewMatrix()
{
    toTarget.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    toTarget.y = sin(glm::radians(pitch));
    toTarget.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    toTarget = glm::normalize(toTarget) * radius;

    return glm::lookAt(target - toTarget, target, upDirection);
}


void Camera::move(std::pair<bool, wxPoint> mouseInfo)
{
    if (!mouseInfo.first)
    {
        mouseMovingPreviousFrame = false;
        return;
    }
    
    if (mouseMovingPreviousFrame)
    {
        int xMove = previousMousePos.x - mouseInfo.second.x;
        int yMove = previousMousePos.y - mouseInfo.second.y;

        std::cout << "x: " << xMove << " y: " << yMove << " yaw: " << yaw << " pitch: " << pitch << std::endl;

        yaw += mouseSensitivity * xMove;
        pitch += mouseSensitivity * yMove;
    }
    
    previousMousePos = mouseInfo.second;
    mouseMovingPreviousFrame = true;
}
