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

    glEnable(GL_DEPTH_TEST);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders->useProgram();

    camera->move(parentCanvas->getCameraMouseInfo());

    setUniformMatrix(camera->viewMatrix(), "view");
    setUniformMatrix(camera->projectionMatrix(
        parentCanvas->viewportAspectRatio()), "projection");

    for (auto it = objects.begin(); it != objects.end(); it++)
        if ((*it)->show)
            (*it)->draw();
        
    // Nvidia cards spit out performance warnings without this call
    // https://stackoverflow.com/a/15079431
    glUseProgram(0);

    // CPU waits for the GPU to execute everything
    glFinish();
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


void GraphicsManager::newObject(std::string file)
{
    std::vector<std::vector<std::string>> data = parseFile(file);

    std::string keyword;
    std::vector<std::tuple<GLuint, GLuint, GLuint>> faceData;

    std::shared_ptr<std::vector<GLfloat>> vertices(new std::vector<GLfloat>());
    std::shared_ptr<std::vector<GLuint>> indices(new std::vector<GLuint>());

    std::string name = "New Object";

    try
    {
        for (std::vector<std::string> line : data)
        {
            keyword = line.front();
            
            if (keyword == "v")
            {
                // keyword x y z
                if (line.size() != 4)
                    throw std::invalid_argument("Incorrect number of params");
                
                for (size_t i = 1; i < line.size(); i++)
                    vertices->push_back(std::stof(line[i]));
            }
            else if (keyword == "f")
            {
                // keyword vertex1/texture1/normal1 vertex2/texture2/normal2...
                if ((line.size() - 1) % 3 != 0 || line.size() == 1)
                    throw std::invalid_argument("The params are missing");
                
                faceData = parseFace(vertices->size(), line);

                if (line.size() > 4)
                    triangulate(&faceData, vertices);
                
                for (std::tuple<GLuint, GLuint, GLuint> point : faceData)
                    indices->push_back(std::get<0>(point));
            }
            else if (keyword == "o")
            {
                // keyword partofname1 partofname2...
                if (line.size() == 1)
                    throw std::invalid_argument("The name is missing");

                name.clear();
                for (size_t i = 1; i < line.size(); i++)
                    name += line[i];
            }
            else
            {
                sendToLog("Unknown keyword: " + keyword);
            }
        }
    }
    catch (std::invalid_argument& exception)
    {
        sendToLog("Object loading error: " + std::string(exception.what()));
        return;
    }

    objects.push_back(std::make_unique<Object>(this, textures, name,
    vertices->data(), vertices->size() * sizeof(GLfloat),
    indices->data(), indices->size() * sizeof(GLfloat)));
    
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


std::vector<std::vector<std::string>> GraphicsManager::parseFile(
    std::string name)
{
    std::ifstream fileStream(name);

    std::vector<std::vector<std::string>> fileVector;

    std::string tempLine, tempSegment;
    std::stringstream lineStream;
    std::vector<std::string> lineVector;
    
    bool backSlash;

    // handle additional spaces, lines, comments(#) and line splits(\)
    // https://stackoverflow.com/a/7868998 - reading line by line
    while (std::getline(fileStream, tempLine))
    {   
        backSlash = !lineVector.empty();

        lineStream << tempLine;

        while(std::getline(lineStream, tempSegment, ' '))
        {
            if (tempSegment.front() == '#')
            {
                lineStream.ignore(std::numeric_limits<std::streamsize>::max());
                break;
            }

            if (!tempSegment.empty())
                lineVector.push_back(tempSegment);
        }
        
        lineStream.clear();

        if (lineVector.empty() || lineVector.back().back() == '\\')
            continue;

        if (backSlash)
            for (size_t i = 0; i < lineVector.size(); i++)
                if (lineVector[i].back() == '\\')
                    {
                        lineVector[i].pop_back();
                        if (lineVector[i].empty())
                            lineVector.erase(lineVector.begin() + i);
                    }
        
        fileVector.push_back(lineVector);
        lineVector.clear();
    }

    return fileVector;
}


std::vector<std::tuple<GLuint, GLuint, GLuint>> GraphicsManager::parseFace(
    size_t vertices, std::vector<std::string> data)
{
    // first tuple signals if the data has texture coords, the second - normals
    std::vector<std::tuple<GLuint, GLuint, GLuint>> ret;

    std::stringstream valueStream;
    std::string tempValue;

    int dataIdx, saveValue;

    for (size_t i = 1; i < data.size(); i++)
    {
        valueStream << data[i];
        dataIdx = 0;
        ret.push_back(std::make_tuple(0, 0, 0));

        while (std::getline(valueStream, tempValue, '\\'))
        {
            if (tempValue.empty())
                continue;
            
            saveValue = std::stoi(tempValue);

            // faces can be indexed negatively, and are 1-based -> shifted by -1
            if (saveValue < 0)
                saveValue = vertices + saveValue;
            else
                saveValue--;

            switch (dataIdx)
            {
                case 0:
                    std::get<0>(ret[i - 1]) = saveValue;
                    break;
                
                case 1:
                    std::get<1>(ret[i - 1]) = saveValue;
                    break;
                
                case 2:
                    std::get<2>(ret[i - 1]) = saveValue;
                    break;
            }
            dataIdx++;
        }
        valueStream.clear();
    }

    return ret;
}


// using Bowyer-Watson algorithm
// https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm
void GraphicsManager::triangulate(
    std::vector<std::tuple<GLuint, GLuint, GLuint>>* indices,
    std::shared_ptr<std::vector<GLfloat>> vertices)
{
    // TODO: split bigger polygons into triangles
    throw std::invalid_argument("Triangulation does not work yet");
    // map is used to index the index list to make the final one
    std::vector<unsigned int> map;

    // first rotate the plane so it is flush with x and y axes

    std::tuple<glm::vec3, glm::vec3, glm::vec3> superTriangle;

    for (auto vertex : *vertices)
    {

    }

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
