#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"

#include <wx/timer.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <chrono>
#include <memory>

#ifdef DEBUG
    #include "GLDebugMessageCallback.h"
    #include <sstream>
#endif /* DEBUG */

class MainFrame;
class Canvas;
class ShaderManager;
class TextureManager;
class VertexBuffer;
class ElementBuffer;
class VertexArray;
class Camera;
class Object;


class GraphicsManager
{
public:
    GraphicsManager(Canvas* parent);
    ~GraphicsManager();

    TextureManager* getTexManagerPtr() { return textures;}
    GLuint getShadersID();
    void render();
    void sendToLog(std::string message);
    void setUniformMatrix(glm::mat4 mat, const char* name);

private:
    Canvas* parentCanvas;
    ShaderManager* shaders;
    TextureManager* textures;
    Camera* camera;
    std::vector<std::unique_ptr<Object>> objects;
};


// https://wiki.wxwidgets.org/Making_a_render_loop
// https://stackoverflow.com/a/87333
// https://stackoverflow.com/a/27739925
class RenderTimer : public wxTimer
{
public:
    RenderTimer(MainFrame* parent, int field, Canvas* canvas);

    // function which is periodically triggered
    void Notify();

private:
    MainFrame* parentFrame;
    int statusBarField;
    Canvas* renderCanvas;
    const float FPSSmoothing = 0.9f;
    std::chrono::steady_clock::time_point lastTick;
    float FPS = 0.0f;
    
    void calculateFPS();
};


class Camera
{
public:
    glm::mat4 viewMatrix();
    glm::mat4 projectionMatrix(float aspectRatio)
        {return glm::perspective(glm::radians(fov), aspectRatio, 
            closeClipBorder, farClipBorder);}
    void move(std::pair<bool, wxPoint> mouseInfo);

private:
    bool mouseMovingPreviousFrame = false;
    wxPoint previousMousePos;
    float mouseSensitivity = 0.1f;

    float yaw = -90.0f;
    float pitch = 0.0f;
    float radius = 10.0f;
    
    float fov = 45.0f;
    float closeClipBorder = 0.1f;
    float farClipBorder = 100.0f;

    glm::vec3 toTarget;
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
};


#endif /* GRAPHICS_HPP_ */
