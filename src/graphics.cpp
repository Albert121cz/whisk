#include "graphics.hpp"

// https://github.com/VictorGordan/opengl-tutorials
// https://learnopengl.com/

GLfloat testVertices[] =
{
    // positions
     0.5f,  0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
};

GLuint testIndices[] =
{
    0, 1, 2,
    0, 2, 3,
};


GraphicsManager::GraphicsManager(Canvas* parent) : parentCanvas(parent)
{
    #ifdef DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(oglDebug::GLDebugMessageCallback, NULL);
    #endif /* DEBUG */

    if (WGLEW_EXT_swap_control_tear)
        wglSwapIntervalEXT(-1);
    else
        wglSwapIntervalEXT(1);

    shaders = new ShaderManager(this);
    shaders->addShader("default.vert");
    shaders->addShader("default.frag");
    shaders->linkProgram();

    textures = new TextureManager(this);

    camera = new Camera();

    objects.push_back(std::make_unique<Object>(this, textures, testVertices,
        sizeof(testVertices), testIndices, sizeof(testIndices)));
}


GraphicsManager::~GraphicsManager()
{
    delete shaders;
    delete textures;
    delete camera;
}


GLuint GraphicsManager::getShadersID() 
{
    return shaders->getID();
}


void GraphicsManager::render()
{
    glClearColor(0.135f, 0.135f, 0.135f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shaders->useProgram();

    camera->move(parentCanvas->getCameraMouseInfo());

    setUniformMatrix(camera->viewMatrix(), "view");
    setUniformMatrix(camera->projectionMatrix(parentCanvas->viewportAspectRatio()), "projection");

    for (auto it = objects.begin(); it != objects.end(); it++)
        (*it)->draw();
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

        // std::cout << "x: " << xMove << " y: " << yMove << " yaw: " << yaw << " pitch: " << pitch << std::endl;

        yaw += mouseSensitivity * xMove;
        pitch += mouseSensitivity * yMove;
    }

    previousMousePos = mouseInfo.second;
    mouseMovingPreviousFrame = true;
}
