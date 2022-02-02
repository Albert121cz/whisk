#ifndef MAIN_HPP_
#define MAIN_HPP_

#include "graphics.hpp"
#include "shaders.hpp"
#include "vertices.hpp"

#ifdef DEBUG
    #include <iostream>
#endif

#include <wx/image.h>
#include <wx/wfstream.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>
#include <GL/glew.h>
#include <memory>
#include <vector>

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 6


class MainFrame;
class Canvas;
class RenderTimer;
class GraphicsManager;


class App : public wxApp
{
public:
    virtual bool OnInit();

private:
    MainFrame* frame;
};

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MainFrame();
    bool openGLInitialized();

private:
    #ifdef DEBUG
        wxLog* logger;
    #endif /* DEBUG */
    Canvas* canvas;
    RenderTimer* timer = nullptr;

    void onObjLoad(wxCommandEvent&);
    void onTexLoad(wxCommandEvent&);
    void onAbout(wxCommandEvent&);
    void onExit(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};

class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);
    ~Canvas() {delete wxGLCtx;}

    bool wxGLCtxExists() {return wxGLCtx != NULL;}
    void flip();
    void addTex(const unsigned char* data, int width, int height);
    float viewportAspectRatio();
    void log(std::string str);
    bool extCheck(std::pair<bool, std::string> in);
    std::pair<bool, wxPoint> getCameraMouseInfo()
        {return std::make_pair(cameraMoving, wxGetMousePosition());}

private:
    MainFrame* parent_ptr;
    wxGLContext* wxGLCtx = nullptr;
    std::unique_ptr<GraphicsManager> graphicsManager;
    bool debuggingExt = false;
    std::pair<int, int> viewportDims;
    bool cameraMoving = false;
    wxPoint mousePos;
    
    void onRender(wxCommandEvent&);
    void onPaint(wxPaintEvent&);
    void onSize(wxSizeEvent&);
    void onLeavingWindow(wxMouseEvent&) {cameraMoving = false;}
    void onRMBDown(wxMouseEvent&);
    void onRMBUp(wxMouseEvent&) {cameraMoving = false;}


    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD_OBJ,
    LOAD_TEX
};


#endif /* MAIN_HPP_ */
