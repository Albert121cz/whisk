#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include "main.hpp"

#include <wx/timer.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#ifdef DEBUG
    #include <sstream>
#endif /* DEBUG */

class Canvas;
class ShaderManager;
class TextureManager;
class VertexBuffer;
class ElementBuffer;
class VertexArray;
class Camera;


class GraphicsManager
{
public:
    GraphicsManager(Canvas* parent);
    ~GraphicsManager();

    TextureManager* getTexManagerPtr() { return textures;}
    void render();
    void oglErrorCheck(int cause);
    void sendToLog(std::string message);

private:
    Canvas* parentCanvas;
    ShaderManager* shaders;
    TextureManager* textures;
    VertexBuffer* vertexBuffer;
    ElementBuffer* elementBuffer;
    VertexArray* vertexArray;
    Camera* camera;

    void setUniformMatrix(glm::mat4 mat, const char* name);
};


// this solution is only temporary, the correct way is to use ogl framebuffers
// https://wiki.wxwidgets.org/Making_a_render_loop
class RenderTimer : public wxTimer
{
public:
    RenderTimer(Canvas* parent);

    // function which is periodically triggered
    void Notify();

private:
    Canvas* parentCanvas;
};


class Camera
{
public:
    glm::mat4 viewMatrix()
        {return glm::lookAt(target - toTarget, target, upDirection);}
    glm::mat4 projectionMatrix(float aspectRatio)
        {return glm::perspective(glm::radians(fov), aspectRatio, 
            closeClipBorder, farClipBorder);}

private:
    float fov = 45.0f;
    float closeClipBorder = 0.1f;
    float farClipBorder = 100.0f;

    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 toTarget = glm::vec3(0.0f, 0.0f, -4.0f);
    glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
};


enum ErrorCause
{
    SHADER_CREATE,
    PROGRAM_LINK,
    PROGRAM_USE,
    BUFFER_LOAD,
    TEX_LOAD,
    ARRAY_ENABLE,
    VERTEX_ATTRIB,
    DEL
};


#endif /* GRAPHICS_HPP_ */
