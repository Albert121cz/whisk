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
#include <wx/laywin.h>
#include <wx/wx.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <memory>
#include <vector>
#include <chrono>

// must be defined for wxCheckListBox to work
#define wxUSE_OWNER_DRAWN 1

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 6


class MainFrame;
class ObjectPanel;
class ObjectButtonPanel;
class RenameFrame;
class Canvas;
class GraphicsManager;
class ListRefreshTimer;


class App : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    MainFrame* frame;
    ObjectPanel* objectFrame;
    wxLayoutAlgorithm* layoutAlgorithm;
};


class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    bool openGLInitialized();

private:
    #ifdef DEBUG
        wxLog* logger;
    #endif /* DEBUG */
    Canvas* canvas;
    ObjectPanel* objects;

    void onObjLoad(wxCommandEvent&);
    void onTexLoad(wxCommandEvent&);
    void onAbout(wxCommandEvent&);
    void onExit(wxCommandEvent&);
    void onClose(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};


class ObjectPanel : public wxPanel
{
public:
    ObjectPanel(MainFrame* parent, std::shared_ptr<GraphicsManager> manager);
    ~ObjectPanel();
    
private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    ObjectButtonPanel* buttons;
    wxCheckListBox* listbox;
    ListRefreshTimer* timer;

    void onCheckBox(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};


class ListRefreshTimer : public wxTimer
{
public:
    ListRefreshTimer(std::shared_ptr<GraphicsManager> manager,
        wxCheckListBox* list);
    ~ListRefreshTimer() {Stop();}
    virtual void Notify() override;

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    wxCheckListBox* listbox;
    wxArrayString names;
};


class ObjectButtonPanel : public wxPanel
{
public:
    ObjectButtonPanel(std::shared_ptr<GraphicsManager> manager,
        wxPanel* parentPanel, wxCheckListBox* target);

private:
    MainFrame* parentFrame;
    std::shared_ptr<GraphicsManager> graphicsManager;
    wxCheckListBox* targetListbox;

    void onNew(wxCommandEvent&);
    void onRename(wxCommandEvent&);
    void onDuplicate(wxCommandEvent&);
    void onDelete(wxCommandEvent&);

    enum buttonEvents
    {
        ID_RENAME,
        ID_DUPLICATE
    };

    wxDECLARE_EVENT_TABLE();
};


class RenameFrame : public wxFrame
{
public:
    RenameFrame(wxWindow* parent, std::shared_ptr<GraphicsManager> manager,
        int idx);

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    wxTextCtrl* textField;
    int objIdx;

    void onEnter(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};


class RenameFrameButtonPanel : public wxPanel
{
public:
    RenameFrameButtonPanel(RenameFrame* parent);

private:
    RenameFrame* parentFrame;

    void onOk(wxCommandEvent&);
    void onCancel(wxCommandEvent&) {parentFrame->Close();}

    wxDECLARE_EVENT_TABLE();
};


class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);
    ~Canvas() {done = true; delete wxGLCtx;}

    bool wxGLCtxExists() {return wxGLCtx != NULL;}
    bool graphicsManagerExists() {return (graphicsManager) ? true : false;}
    void flip();
    void addTex(const unsigned char* data, int width, int height);
    float viewportAspectRatio();
    void log(std::string str);
    bool extCheck(std::pair<bool, std::string> in);
    std::shared_ptr<GraphicsManager> getGraphicsManager()
        {return graphicsManager;}
    std::pair<bool, wxPoint> getCameraMouseInfo()
        {return std::make_pair(cameraMoving, wxGetMousePosition());}
    
    bool done = false;

private:
    MainFrame* parentFrame;
    wxGLContext* wxGLCtx = nullptr;
    std::shared_ptr<GraphicsManager> graphicsManager = {};
    bool debuggingExt = false;
    std::pair<int, int> viewportDims;
    bool cameraMoving = false;
    wxPoint mousePos;
    wxEvent* renderEvent;
    std::chrono::steady_clock::time_point lastFlip;
    const float FPSSmoothing = 0.9f;
    float FPS = 0.0f;
    
    void onRender(wxCommandEvent&);
    void onClose(wxCloseEvent&);
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
