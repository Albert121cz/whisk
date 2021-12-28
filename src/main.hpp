#ifndef MAIN_HPP_
#define MAIN_HPP_

#ifdef DEBUG
    #include <iostream>
#endif

#include <wx/wxprec.h>
#include <wx/wfstream.h>
#include <wx/glcanvas.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

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
    // void size(wxSizeEvent& event);

private:
    MainFrame* parent_ptr;
    wxGLContext* glctx_ptr;
    // myOGLManager* m_oglManager;
    // int m_winHeight; // We use this var to know if we have been sized


    wxDECLARE_EVENT_TABLE();
};

enum Event
{
    LOAD
};


#endif // MAIN_HPP_