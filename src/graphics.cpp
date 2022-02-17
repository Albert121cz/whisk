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

    objects.push_back(std::make_unique<Object>(this, textures, "Plane",
        testVertices, sizeof(testVertices), testIndices, sizeof(testIndices)));
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
    setUniformMatrix(camera->projectionMatrix(
        parentCanvas->viewportAspectRatio()), "projection");

    for (auto it = objects.begin(); it != objects.end(); it++)
        if ((*it)->show)
            (*it)->draw();
}


#ifdef DEBUG
    void GraphicsManager::sendToLog(std::string message)
    {
        parentCanvas->log(message);
    }
#endif /* DEBUG */


void GraphicsManager::setUniformMatrix(glm::mat4 mat, const char* name)
{
    int location = glGetUniformLocation(shaders->getID(), name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}


void GraphicsManager::addObject(std::string name, GLfloat* vert, size_t vertSize,
    GLuint* ind, size_t indSize)
{
    objects.push_back(std::make_unique<Object>(this, textures, name,
        vert, vertSize, ind, indSize));
    
    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object added: " << name;
        sendToLog(messageStream.str());
    #endif /* DEBUG */
}


void GraphicsManager::renameObject(int idx, std::string newName)
{
    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object name changed: " << objects[idx]->objectName
            << " -> " << newName;
        sendToLog(messageStream.str());
    #endif /* DEBUG */

    objects[idx]->objectName = newName;
}


void GraphicsManager::duplicateObject(int idx)
{
    // integer is cast to size_t, so the compiler doesn't flag this with
    // a warning; GetSelection() from wxCheckListBox returns int anyway
    if (static_cast<size_t>(idx) >= objects.size())
        return;

    int newObjectIdx = idx + 1;
    
    objects.insert(objects.begin() + newObjectIdx,
        std::make_unique<Object>(*objects[idx]));
    
    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object duplicated: " << objects[idx]->objectName;
        sendToLog(messageStream.str());
    #endif /* DEBUG */
}


void GraphicsManager::deleteObject(int idx)
{
    if (static_cast<size_t>(idx) >= objects.size())
        return;

    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object deleted: " << objects[idx]->objectName;
        sendToLog(messageStream.str());
    #endif /* DEBUG */

    objects.erase(objects.begin() + idx);
}


void GraphicsManager::showOrHideObject(int idx)
{
    if (static_cast<size_t>(idx) >= objects.size())
        return;

    if (objects[idx]->show)
    {
        objects[idx]->show = false;

        #ifdef DEBUG
            std::ostringstream messageStream;
            messageStream << "Object hid: " << objects[idx]->objectName;
            sendToLog(messageStream.str());
        #endif /* DEBUG */
    }
    else
    {
        objects[idx]->show = true;

        #ifdef DEBUG
            std::ostringstream messageStream;
            messageStream << "Object showed: " << objects[idx]->objectName;
            sendToLog(messageStream.str());
        #endif /* DEBUG */
    }
}


bool GraphicsManager::getObjectShow(int idx)
{
    return objects[idx]->show;
}


std::string GraphicsManager::getObjectName(int idx)
{
    return objects[idx]->objectName;
}


glm::vec3* GraphicsManager::getObjectPosVec(int idx)
{
    return &objects[idx]->position;
}


glm::vec3* GraphicsManager::getObjectRotVec(int idx)
{
    return &objects[idx]->rotation;
}


glm::vec3* GraphicsManager::getObjectSize(int idx)
{
    return &objects[idx]->size;
}


std::vector<std::string> GraphicsManager::getAllObjectNames()
{
    std::vector<std::string> names;

    for (auto it = objects.begin(); it != objects.end(); it++)
        names.push_back((*it)->objectName);
    
    return names;
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
