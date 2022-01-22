#ifndef MAIN_HPP_
#define MAIN_HPP_

#include "graphics.hpp"
#include "shaders.hpp"
#include "vertices.hpp"

#ifdef DEBUG
    #include <iostream>
#endif

#include <wx/wfstream.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>
#include <GL/glew.h>
#include <memory>

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 6

class App : public wxApp
{
public:
    virtual bool OnInit();
};

class Canvas;
class RenderTimer;

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    bool opengl_initialized();

private:
    #ifdef DEBUG
        wxLog* logger_ptr;
    #endif /* DEBUG */
    Canvas* canvas_ptr;

    void onLoad(wxCommandEvent&);
    void onAbout(wxCommandEvent&);
    void onExit(wxCommandEvent&) {Close(true);}

    wxDECLARE_EVENT_TABLE();
};

class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);

    bool glctx_exists() {return glctx_ptr != NULL;}
    void flip();
    
    void onPaint(wxPaintEvent&);
    void onSize(wxSizeEvent&);
    void log(std::string str);

private:
    bool firstPaint = true;
    MainFrame* parent_ptr;
    wxGLContext* glctx_ptr;
    RenderTimer* timer;
    std::unique_ptr<GraphicsManager> graphicsManager;

    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD
};


#endif /* MAIN_HPP_ */
