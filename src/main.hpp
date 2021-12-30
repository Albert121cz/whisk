#ifndef MAIN_HPP_
#define MAIN_HPP_

#include "graphics.hpp"

#ifdef DEBUG
    #include <iostream>
#endif

#include <wx/wfstream.h>
#include <wx/glcanvas.h>
#include <wx/wx.h>
#include <GL/glew.h>

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 6

class App : public wxApp
{
public:
    virtual bool OnInit();
};

class Canvas;

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    bool opengl_initialized();

private:
    #ifdef DEBUG
        wxLog* logger_ptr;
    #endif
    Canvas* canvas_ptr;

    void load(wxCommandEvent& event);
    void about(wxCommandEvent& event);
    void exit(wxCommandEvent& event) {Close(true);}

    wxDECLARE_EVENT_TABLE();
};

class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);
    bool glctx_exists() {return glctx_ptr != NULL;}
    
    void paint(wxPaintEvent& event);
    std::pair<int, int>getCtxSize();
    // TODO: make useful error reporting
    void logError(int err) {wxLogVerbose("%i", err);};
    // void size(wxSizeEvent& event);

private:
    MainFrame* parent_ptr;
    wxGLContext* glctx_ptr;
    // std::unique_ptr<GraphicsManager> graphicsManager;
    GraphicsManager* graphicsManager;

    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD
};


#endif /* MAIN_HPP_ */