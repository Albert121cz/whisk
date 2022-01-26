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
    virtual int OnExit();

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

    bool wxGLCtxExists() {return wxGLCtx != NULL;}
    void flip();
    void addTex(const unsigned char* data, int width, int height);
    float viewportAspectRatio();
    
    void onPaint(wxPaintEvent&);
    void onSize(wxSizeEvent&);
    void log(std::string str);

private:
    MainFrame* parent_ptr;
    wxGLContext* wxGLCtx;
    std::unique_ptr<GraphicsManager> graphicsManager;
    std::pair<int, int> viewportDims;

    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD_OBJ,
    LOAD_TEX
};


#endif /* MAIN_HPP_ */
