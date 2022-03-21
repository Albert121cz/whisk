#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"

#include <wx/timer.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <list>

#ifdef DEBUG
    #include "GLDebugMessageCallback.h"
    #include <sstream>
#endif /* DEBUG */

class MainFrame;
class Canvas;
class ShaderManager;
class VertexBuffer;
class ElementBuffer;
class VertexArray;
class Camera;
class Object;
class Texture;
struct MouseInfo;


class GraphicsManager
{
public:
    GraphicsManager(Canvas* parent);
    ~GraphicsManager();

    GLuint getShadersID();
    bool getShadersCompiled();
    void render();
    void sendToLog(std::string message);
    void setUniformMatrix(glm::mat4 mat, const char* name);
    void newObject(std::string file, size_t startLine = 0,
        std::shared_ptr<std::vector<std::vector<std::string>>> data = nullptr,
        std::shared_ptr<std::vector<GLfloat>> vertices = nullptr,
        std::shared_ptr<std::vector<GLfloat>> texVertices = nullptr,
        std::shared_ptr<std::vector<GLfloat>> normals = nullptr);
    void renameObject(int idx, std::string newName);
    void setObjectColor(int idx, GLfloat r, GLfloat g, GLfloat b);
    void setObjectTex(int idx, std::shared_ptr<Texture> tex);
    void duplicateObject(int idx);
    void deleteObject(int idx);
    void showOrHideObject(int idx);
    bool getObjectShow(int idx);
    std::string getObjectName(int idx);
    std::tuple<GLfloat, GLfloat, GLfloat> getObjectColor(int idx);
    glm::vec3* getObjectPosVec(int idx);
    glm::vec3* getObjectRotVec(int idx);
    glm::vec3* getObjectSize(int idx);
    int* getObjectMode(int idx);
    std::vector<std::string> getAllObjectNames();
    void addTexture(const unsigned char* data, int width, int height,
        std::string name);
    void deleteTexture(int idx);
    std::shared_ptr<Texture> getTexPtr(int idx);
    std::vector<std::string> getAllTextureNames();

private:
    Canvas* parentCanvas;
    ShaderManager* shaders;
    Camera* camera;

    bool shadersCompiled;

    std::vector<std::unique_ptr<Object>> objects;
    std::vector<std::shared_ptr<Texture>> textures;

    glm::vec3 lightColor;
    glm::vec3 cameraToLight;

    void setUniformVector(glm::vec3 vec, const char* name);
    std::vector<std::vector<std::string>> parseFile(std::string name);
    std::vector<std::tuple<int, int, int>> parseFace(size_t vertices,
        std::vector<std::string> data);
    void triangulate(std::vector<std::tuple<int, int, int>>* indices,
        std::shared_ptr<std::vector<GLfloat>> allVertices,
        glm::vec3* normalVec);
};


class Camera
{
public:
    Camera();

    glm::mat4 viewMatrix();
    glm::mat4 projectionMatrix(float aspectRatio);
    void move(MouseInfo info);
    glm::vec3 getPos();

private:
    bool cameraSpinningPrevFrame;
    bool cameraMovingPrevFrame;
    wxPoint prevMousePos;
    float spinSensitivity;
    float moveSensitivity;
    float scrollSensitivity;
    int previousWheelPos = 0;

    float horizMove;
    float vertiMove;

    float yaw;
    float pitch;
    float radius;
    
    float fov;
    float closeClipBorder;
    float farClipBorder;

    glm::vec3 toTarget;
    glm::vec3 target;
    glm::vec3 upDirection;
};


#endif /* GRAPHICS_HPP_ */
