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
#include <wx/spinctrl.h>
#include <wx/wx.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <glm/glm.hpp>
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
class ObjectSettings;
class Canvas;
class GraphicsManager;
class SidePanelRefreshTimer;


class App : public wxApp
{
public:
    virtual bool OnInit() override;

private:
    MainFrame* frame;
    ObjectPanel* objectFrame;
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

    enum Event
    {
        LOAD_OBJ,
        LOAD_TEX
    };

    void onObjLoad(wxCommandEvent&);
    void onTexLoad(wxCommandEvent&);
    void onAbout(wxCommandEvent&);
    void onExit(wxCommandEvent&);
    void onClose(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};


class SidePanel : public wxPanel
{
public:
    SidePanel(MainFrame* parent, std::shared_ptr<GraphicsManager> manager);
    ~SidePanel();

private:
    SidePanelRefreshTimer* timer;
};


class ObjectPanel : public wxPanel
{
public:
    ObjectPanel(SidePanel* parent, MainFrame* main,
        std::shared_ptr<GraphicsManager> manager);

    wxCheckListBox* getListbox();

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    ObjectButtonPanel* buttons;
    wxCheckListBox* listbox;
    SidePanelRefreshTimer* timer;

    void onCheckBox(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};


class SidePanelRefreshTimer : public wxTimer
{
public:
    SidePanelRefreshTimer(std::shared_ptr<GraphicsManager> manager,
        ObjectSettings* settings, wxCheckListBox* list);
    ~SidePanelRefreshTimer() {Stop();}
    virtual void Notify() override;

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    ObjectSettings* objectSettings;
    wxCheckListBox* listbox;
    wxArrayString names;
    int lastSelected = wxNOT_FOUND;
};


class ObjectButtonPanel : public wxPanel
{
public:
    ObjectButtonPanel(std::shared_ptr<GraphicsManager> manager,
        wxPanel* parentPanel, MainFrame* main, wxCheckListBox* target);

private:
    MainFrame* mainFrame;
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
    RenameFrame(MainFrame* parent, std::shared_ptr<GraphicsManager> manager,
        int idx);
    ~RenameFrame();

private:
    MainFrame* mainFrame;
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


class ObjectSettings : public wxPanel
{
public:
    ObjectSettings(wxPanel* parent, wxCheckListBox* list,
        std::shared_ptr<GraphicsManager> manager);

private:
    wxCheckListBox* listbox;
    std::shared_ptr<GraphicsManager> graphicsManager;
    std::vector<wxSpinCtrlDouble*> textFields;

    void onRefresh(wxCommandEvent&);
    void onEnter(wxCommandEvent&);
    void onChange(wxSpinDoubleEvent& event);

    enum TextFieldsIdx
    {
        POS_X = 0,
        POS_Y = 1,
        POS_Z = 2,
        ROT_X = 3,
        ROT_Y = 4,
        ROT_Z = 5,
        SIZE = 6
    };

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

#endif /* MAIN_HPP_ */
