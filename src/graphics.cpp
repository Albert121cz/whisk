#include "graphics.hpp"

// https://github.com/VictorGordan/opengl-tutorials
// https://learnopengl.com/


GraphicsManager::GraphicsManager(Canvas* parent) : parentCanvas(parent)
{
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    cameraToLight = glm::vec3(2.0f, 2.0f, -2.0f);

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
}


GraphicsManager::~GraphicsManager()
{
    delete shaders;
    delete textures;
    delete camera;
}


TextureManager* GraphicsManager::getTexManagerPtr() const
{
    return textures;
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

    camera->move(parentCanvas->getMouseInfo());

    setUniformMatrix(camera->viewMatrix(), "view");
    setUniformMatrix(camera->projectionMatrix(
        parentCanvas->viewportAspectRatio()), "projection");
    
    setUniformVector(lightColor, "lightColor");
    setUniformVector(camera->getPos() + cameraToLight, "lightPos");
    // setUniformVector(glm::vec3(2.0, 2.0, 2.0), "lightPos");

    for (auto it = objects.begin(); it != objects.end(); it++)
        if ((*it)->show)
            (*it)->draw();
        
    // Nvidia cards spit out performance warnings without this call
    // https://stackoverflow.com/a/15079431
    glUseProgram(0);

    // CPU waits for the GPU to execute everything
    glFinish();
}


void GraphicsManager::sendToLog(std::string message)
{
    parentCanvas->log(message);
}


void GraphicsManager::setUniformMatrix(glm::mat4 mat, const char* name)
{
    int location = glGetUniformLocation(shaders->getID(), name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}


void GraphicsManager::setUniformVector(glm::vec3 vec, const char* name)
{
    int location = glGetUniformLocation(shaders->getID(), name);
    glUniform3f(location, vec.x, vec.y, vec.z);
}


void GraphicsManager::newObject(std::string file, size_t startLine,
    std::shared_ptr<std::vector<std::vector<std::string>>> data,
    std::shared_ptr<std::vector<GLfloat>> vertices,
    std::shared_ptr<std::vector<GLfloat>> texVertices,
    std::shared_ptr<std::vector<GLfloat>> normals)
{
    if (data == nullptr)
        data = std::make_shared<std::vector<std::vector<std::string>>>(
            parseFile(file));

    std::string keyword;
    std::vector<std::tuple<int, int, int>> faceData;
    std::pair<int, int> lineData;

    if (vertices == nullptr)
        vertices = std::make_shared<std::vector<GLfloat>>();

    if (texVertices == nullptr)
        texVertices = std::make_shared<std::vector<GLfloat>>();

    if (normals == nullptr)
        normals = std::make_shared<std::vector<GLfloat>>();

    std::shared_ptr<std::vector<GLfloat>> finalLineVertices =
        std::make_shared<std::vector<GLfloat>>();
    std::shared_ptr<std::vector<GLfloat>> finalVertices =
        std::make_shared<std::vector<GLfloat>>();
    std::shared_ptr<std::vector<GLfloat>> finalTextures =
        std::make_shared<std::vector<GLfloat>>();
    std::shared_ptr<std::vector<GLfloat>> finalNormals =
        std::make_shared<std::vector<GLfloat>>();

    std::string name = "New Object";
    bool nameModified = false;

    std::vector<std::string> line;

    try
    {
        for (size_t lineIdx = startLine; lineIdx < data->size(); lineIdx++)
        {
            line = data->at(lineIdx);
            keyword = line.front();
            
            if (keyword == "v")
            {
                // v x-coord y-coord z-coord
                if (line.size() != 4)
                    throw std::invalid_argument("Incorrect number of params");
                
                for (size_t i = 1; i < line.size(); i++)
                    vertices->push_back(std::stof(line[i]));
            }
            else if (keyword == "vt")
            {
                // vt x-coord y-coord
                if (line.size() != 3)
                    throw std::invalid_argument("Incorrect number of params");
                
                for (size_t i = 1; i < line.size(); i++)
                    texVertices->push_back(std::stof(line[i]));
            }
            else if (keyword == "vn")
            {
                // vn x-coord y-coord z-coord
                if (line.size() != 4)
                    throw std::invalid_argument("Incorrect number of params");
                
                for (size_t i = 1; i < line.size(); i++)
                    normals->push_back(std::stof(line[i]));
            }
            else if (keyword == "f")
            {
                // f vertex1/texture1/normal1 vertex2/texture2/normal2...
                if (line.size() < 4)
                    throw std::invalid_argument("The params are missing");
                
                faceData = parseFace(vertices->size(), line);

                if (line.size() > 4)
                    triangulate(&faceData, vertices);
                
                for (std::tuple<int, int, int> point : faceData)
                {
                    finalVertices->push_back(
                        vertices->at(std::get<0>(point) * 3));
                    finalVertices->push_back(
                        vertices->at(std::get<0>(point) * 3 + 1));
                    finalVertices->push_back(
                        vertices->at(std::get<0>(point) * 3 + 2));

                    if (std::get<1>(point) != -1)
                    {
                        finalTextures->push_back(
                            texVertices->at(std::get<1>(point) * 2));
                        finalTextures->push_back(
                            texVertices->at(std::get<1>(point) * 2 + 1));
                    }
                    
                    if (std::get<2>(point) != -1)
                    {
                        finalNormals->push_back(
                            normals->at(std::get<2>(point) * 3));
                        finalNormals->push_back(
                            normals->at(std::get<2>(point) * 3 + 1));
                        finalNormals->push_back(
                            normals->at(std::get<2>(point) * 3 + 2));
                    }
                }
            }
            else if (keyword == "l")
            {
                // l vertex1 vertex2
                if (line.size() != 3)
                    throw std::invalid_argument("The params are missing");
                
                lineData = std::make_pair(
                    std::stoi(line[1]), std::stoi(line[2]));
                
                for (int vertIdx : {lineData.first, lineData.second})
                {
                    if (vertIdx < 0)
                        vertIdx = vertices->size() + vertIdx;
                    else if (static_cast<size_t>(vertIdx) >= vertices->size())
                        throw std::invalid_argument(
                            "Invalid vertex index in face");
                    else
                        vertIdx--;

                    finalLineVertices->push_back(
                        vertices->at(vertIdx * 3));
                    finalLineVertices->push_back(
                        vertices->at(vertIdx * 3 + 1));
                    finalLineVertices->push_back(
                        vertices->at(vertIdx * 3 + 2));
                }
            }
            else if (keyword == "o")
            {
                // o partofname1 partofname2...
                if (line.size() == 1)
                    throw std::invalid_argument("The name is missing");
                
                if (nameModified)
                {
                    newObject(file, lineIdx, data, vertices, texVertices,
                        normals);
                    break;
                }

                name.clear();
                for (size_t i = 1; i < line.size(); i++)
                    name += line[i];
                nameModified = true;
            }
        }
    }
    catch (std::invalid_argument& exception)
    {
        sendToLog("Object loading error: " + std::string(exception.what()));
        return;
    }

    // the name has to fit inside the wxCheckListBox
    if (name.size() > 24)
        name.resize(24);
    
    // add all line vertices to the end of finalVertices
    finalVertices->insert(finalVertices->end(),
        finalLineVertices->begin(), finalLineVertices->end());

    objects.push_back(std::make_unique<Object>(this, name,
        finalLineVertices->size(), finalVertices, finalTextures,
        finalNormals));
    
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


void GraphicsManager::setObjectColor(int idx, GLfloat r, GLfloat g, GLfloat b)
{
    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object color changed: " << objects[idx]->objectName
            << " -> " << r << "x" << g << "x" << b;
        sendToLog(messageStream.str());
    #endif /* DEBUG */

    objects[idx]->setColor(r, g, b);
    objects[idx]->hasTex = false;
}


void GraphicsManager::setObjectTex(int idx, std::shared_ptr<Texture> tex)
{
    #ifdef DEBUG
        std::ostringstream messageStream;
        messageStream << "Object texture changed: " << objects[idx]->objectName
            << " -> " << tex->textureName;
        sendToLog(messageStream.str());
    #endif /* DEBUG */

    objects[idx]->tex = tex;
    objects[idx]->hasTex = true;
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


std::tuple<GLfloat, GLfloat, GLfloat> GraphicsManager::getObjectColor(int idx)
{
    return objects[idx]->getColor();
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


int* GraphicsManager::getObjectMode(int idx)
{
    return &objects[idx]->renderMode;
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


std::vector<std::tuple<int, int, int>> GraphicsManager::parseFace(
    size_t vertices, std::vector<std::string> data)
{
    // first tuple signals if the data has texture coords, the second - normals
    std::vector<std::tuple<int, int, int>> ret;

    std::stringstream valueStream;
    std::string tempValue;

    int dataIdx, saveValue;

    for (size_t i = 1; i < data.size(); i++)
    {
        valueStream << data[i];
        dataIdx = 0;
        ret.push_back(std::make_tuple(-1, -1, -1));

        while (std::getline(valueStream, tempValue, '/'))
        {
            if (tempValue.empty())
                continue;

            saveValue = std::stoi(tempValue);

            // faces can be indexed negatively, and are 1-based
            if (saveValue < 0)
                saveValue = vertices + saveValue;
            else if (static_cast<size_t>(saveValue) >= vertices)
                throw std::invalid_argument("Invalid vertex index in face");
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


// using ear-clipping method
// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
void GraphicsManager::triangulate(
    std::vector<std::tuple<int, int, int>>* indices,
    std::shared_ptr<std::vector<GLfloat>> allVertices)
{
    struct vertex
    {
        glm::vec3 pos;
        unsigned int idx;
        vertex(glm::vec3 vec, unsigned int index) : pos(vec), idx(index) {}
    };

    std::list<vertex> verticesList;

    for (size_t i = 0; i < indices->size(); i++)
    {
        verticesList.push_back(vertex(glm::vec3(
            allVertices->at(std::get<0>(indices->at(i)) * 3),
            allVertices->at(std::get<0>(indices->at(i)) * 3 + 1),
            allVertices->at(std::get<0>(indices->at(i)) * 3 + 2)), i));
    }
    
    std::list<vertex>::iterator delIt = verticesList.begin();
    std::list<vertex>::iterator checkIt;
    while (delIt != verticesList.end())
    {
        checkIt = verticesList.begin();
        while (checkIt != verticesList.end())
        {
            if (delIt->pos == checkIt->pos && delIt->idx != checkIt->idx)
                checkIt = verticesList.erase(checkIt);
            else
                checkIt++;
        }
        delIt++;
    }

    std::list<vertex>::iterator it = verticesList.begin();

    std::vector<GLuint> map;
    glm::vec3 rotationAxis = glm::normalize(
        glm::cross(it->pos - std::next(it, 1)->pos,
        std::next(it, 2)->pos - std::next(it, 1)->pos));
    
    bool outside, skip;
    std::vector<vertex> triangleVertices;
    std::vector<vertex>::iterator prev, next;
    glm::vec3 vecToPrev, vecToNext, referenceVec, testVec;
    float referenceAngle, testAngle;

    while (verticesList.size() > 3)
    {
        if (it == verticesList.end())
            it = verticesList.begin();

        triangleVertices.clear();
        if (it == verticesList.begin())
            triangleVertices.push_back(*std::prev(verticesList.end(), 1));
        else
            triangleVertices.push_back(*std::prev(it, 1));
        
        triangleVertices.push_back(*it);

        if (it == std::prev(verticesList.end(), 1))
            triangleVertices.push_back(*verticesList.begin());
        else
            triangleVertices.push_back(*std::next(it, 1));

        // test each vertex if it lies inside the triangle
        for (std::list<vertex>::iterator testIt = verticesList.begin();
            testIt != verticesList.end(); testIt++)
        {
            skip = false;
            for (vertex triangle : triangleVertices)
                if (testIt->idx == triangle.idx)
                {
                    skip = true;
                    break;
                }
            if (skip)
                continue;

            outside = false;

            for (auto triangleIt = triangleVertices.begin();
                triangleIt != triangleVertices.end(); triangleIt++)
            {
                if (triangleIt == triangleVertices.begin())
                    // prev = triangleVertices.end() - 1;
                    prev = std::prev(triangleVertices.end(), 1);
                else
                    // prev = triangleIt - 1;
                    prev = std::prev(triangleIt, 1);

                if (triangleIt == triangleVertices.end() - 1)
                    next = triangleVertices.begin();
                else
                    // next = triangleIt + 1;
                    next = std::next(triangleIt, 1);

                vecToPrev = prev->pos - triangleIt->pos;
                vecToNext = next->pos - triangleIt->pos;

                referenceVec = glm::normalize(vecToNext);

                referenceAngle = glm::orientedAngle(referenceVec,
                    glm::normalize(vecToPrev), rotationAxis);
                if (referenceAngle > glm::pi<float>())
                {
                    referenceVec = glm::normalize(vecToPrev);
                    referenceAngle = 2 * glm::pi<float>() - referenceAngle;
                }

                testVec = testIt->pos - triangleIt->pos;

                testAngle = glm::orientedAngle(
                    referenceVec, glm::normalize(testVec), rotationAxis);
                
                if (testAngle > referenceAngle)
                {
                    outside = true;
                    break;
                }
            }

            // if the vert is inside the triangle
            if (!outside)
            {
                it++;
                goto nextIt;
            }
        }

        map.push_back(it->idx);

        if (it == verticesList.begin())
            map.push_back(std::prev(verticesList.end(), 1)->idx);
        else
            map.push_back(std::prev(it, 1)->idx);
        
        if (it == std::prev(verticesList.end(), 1))
            map.push_back(verticesList.begin()->idx);
        else
            map.push_back(std::next(it, 1)->idx);

        it = verticesList.erase(it);

        nextIt:;
    }

    // add the last remaining triangle to map
    if (verticesList.size() == 3)
        for (vertex vert : verticesList)
            map.push_back(vert.idx);

    size_t indicesOriginalSize = indices->size();
    for (GLuint idx : map)
        indices->push_back(indices->at(idx));
    indices->erase(indices->begin(), indices->begin() + indicesOriginalSize);
}


Camera::Camera()
{
    cameraSpinningPrevFrame = false;
    cameraMovingPrevFrame = false;
    spinSensitivity = 0.15f;
    moveSensitivity = 0.01f;
    scrollSensitivity = 0.00005f;

    horizMove = 0.0f;
    vertiMove = 0.0f;

    yaw = -90.0f;
    pitch = 0.0f;
    radius = 10.0f;

    fov = 45.0f;
    closeClipBorder = 0.1f;
    farClipBorder = 100.0f;

    target = glm::vec3(0.0f, 0.0f, 0.0f);
    upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
}


glm::mat4 Camera::viewMatrix()
{
    if (pitch > 90)
        pitch = std::nextafterf(90.0f, 0.0f);
    else if (pitch < -90)
        pitch = -std::nextafterf(90.0f, 0.0f);
    
    if (radius < 0)
        radius = 1.0f;

    toTarget.x = cos(glm::radians(-yaw)) * cos(glm::radians(pitch));
    toTarget.y = sin(glm::radians(pitch));
    toTarget.z = sin(glm::radians(-yaw)) * cos(glm::radians(pitch));
    toTarget = glm::normalize(toTarget) * radius;

    target += glm::normalize(glm::cross(toTarget, upDirection)) * horizMove;
    target -= glm::normalize(glm::cross(
        glm::normalize(glm::cross(toTarget, upDirection)),
        glm::normalize(toTarget))) * vertiMove;

    return glm::lookAt(target - toTarget, target, upDirection);
}


glm::mat4 Camera::projectionMatrix(float aspectRatio)
{
    return glm::perspective(
        glm::radians(fov), aspectRatio, closeClipBorder, farClipBorder);
}


void Camera::move(MouseInfo info)
{
    radius += scrollSensitivity * (previousWheelPos - info.wheelPos);
    previousWheelPos = info.wheelPos;
    
    if (info.spinning)
    {
        if (cameraSpinningPrevFrame)
        {
            yaw += spinSensitivity * (prevMousePos.x - info.mousePos.x);
            pitch += spinSensitivity * (prevMousePos.y - info.mousePos.y);
        }

        prevMousePos = info.mousePos;
        cameraSpinningPrevFrame = true;
    }
    else
        cameraSpinningPrevFrame = false;

    if (info.moving)
    {
        if (cameraMovingPrevFrame)
        {
            horizMove = moveSensitivity * (prevMousePos.x - info.mousePos.x);
            vertiMove = moveSensitivity * (prevMousePos.y - info.mousePos.y);
        }

        prevMousePos = info.mousePos;
        cameraMovingPrevFrame = true;
    }
    else
    {
        horizMove = 0;
        vertiMove = 0;
        cameraMovingPrevFrame = false;
    }
}

glm::vec3 Camera::getPos()
{
    return target - toTarget;
}
