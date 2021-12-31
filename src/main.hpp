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
#include <memory>

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
    #endif /* DEBUG */
    Canvas* canvas_ptr;

    void onLoad(wxCommandEvent& event);
    void onAbout(wxCommandEvent& event);
    void onExit(wxCommandEvent& event) {Close(true);}

    wxDECLARE_EVENT_TABLE();
};

class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);
    bool glctx_exists() {return glctx_ptr != NULL;}
    
    void onPaint(wxPaintEvent& event);
    void onSize(wxSizeEvent& event);
    void oglErrorLog(int cause, int err);

private:
    MainFrame* parent_ptr;
    wxGLContext* glctx_ptr;
    std::unique_ptr<GraphicsManager> graphicsManager;

    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD
};

enum ErrorCause
{
    SHADER_CREATE,
    PROGRAM_LINK,
    PROGRAM_USE,
    BUFFER_LOAD,
    DEL
};


#endif /* MAIN_HPP_ */