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
        if (parentCanvas->getDebuggingExt())
            {
                glEnable(GL_DEBUG_OUTPUT);
                glDebugMessageCallback(oglDebug::GLDebugMessageCallback, NULL);
            }
    #endif /* DEBUG */

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
    delete vertexArray;
    delete vertexBuffer;
    delete elementBuffer;
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


RenderTimer::RenderTimer(MainFrame* parent, int field, Canvas* canvas)
    : parentFrame(parent), statusBarField(field), renderCanvas(canvas)
{
    StartOnce(0);
    lastTick = std::chrono::steady_clock::now();
}


void RenderTimer::Notify()
{
    renderCanvas->flip();
    StartOnce();
    calculateFPS();

    // https://stackoverflow.com/a/40766420
    parentFrame->SetStatusText(
        wxString::Format(wxT("%.1f FPS"), FPS), statusBarField);
}


void RenderTimer::calculateFPS()
{
    std::chrono::steady_clock::time_point currentTick;
    currentTick = std::chrono::steady_clock::now();
    int difference = std::chrono::duration_cast<std::chrono::milliseconds>
        (currentTick - lastTick).count();
    FPS = (FPS * FPSSmoothing) + (1000/difference * (1.0-FPSSmoothing));
    lastTick = currentTick;
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
