#ifndef MAIN_HPP_
#define MAIN_HPP_

#include "graphics.hpp"
#include "shaders.hpp"
#include "vertices.hpp"

#ifdef DEBUG
    #include <iostream>
#endif

#include <wx/image.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/colordlg.h>
#include <wx/wx.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <glm/glm.hpp>
#include <fstream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <regex>

// must be defined for wxCheckListBox to work
#define wxUSE_OWNER_DRAWN 1

#define OGL_MAJOR_VERSION 4
#define OGL_MINOR_VERSION 6


class MainFrame;
class ObjectList;
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
};


class MainFrame : public wxFrame
{
public:
    MainFrame();
    bool openGLInitialized();

private:
    #ifdef DEBUG
        wxLog* logger;
    #endif /* DEBUG */
    Canvas* canvas;

    void onObjLoad(wxCommandEvent&);
    void onAbout(wxCommandEvent&);
    void onExit(wxCommandEvent&);
    void onClose(wxCloseEvent& event);

    enum Event
    {
        LOAD_OBJ
    };

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


class ObjectList : public wxPanel
{
public:
    ObjectList(SidePanel* parent, MainFrame* main,
        std::shared_ptr<GraphicsManager> manager);

    wxCheckListBox* getListbox();

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    ObjectButtonPanel* buttons;
    wxCheckListBox* listbox;

    void onCheckBox(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};


class SidePanelRefreshTimer : public wxTimer
{
public:
    SidePanelRefreshTimer(std::shared_ptr<GraphicsManager> manager,
        ObjectSettings* settings, wxCheckListBox* list);
    ~SidePanelRefreshTimer();
    virtual void Notify() override;

private:
    std::shared_ptr<GraphicsManager> graphicsManager;
    ObjectSettings* objectSettings;
    wxCheckListBox* listbox;
    wxArrayString names;
    int lastSelected;
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
    void onColor(wxCommandEvent&);
    void onTexture(wxCommandEvent&);
    void onDuplicate(wxCommandEvent&);
    void onDelete(wxCommandEvent&);

    enum ButtonID
    {
        ID_RENAME,
        ID_COLOR,
        ID_TEXTURE,
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
    void onCancel(wxCommandEvent&);

    wxDECLARE_EVENT_TABLE();
};


class TextureFrame : public wxFrame
{
public:
    TextureFrame(MainFrame* parent, std::shared_ptr<GraphicsManager> manager,
        int idx);
    ~TextureFrame();

private:
    MainFrame* mainFrame;
    wxListBox* listBox;
};


class TextureFrameButtonPanel : public wxPanel
{
public:
    TextureFrameButtonPanel(TextureFrame* parent, 
    std::shared_ptr<GraphicsManager> manager, wxListBox* target, int idx);

private:
    TextureFrame* parentFrame;
    std::shared_ptr<GraphicsManager> graphicsManager;
    wxListBox* targetListBox;
    int objIdx;

    void onNew(wxCommandEvent&);
    void onDelete(wxCommandEvent&);
    void onOk(wxCommandEvent&);
    void onCancel(wxCommandEvent&);

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
    wxChoice* renderModeChoice;

    void onRefresh(wxCommandEvent&);
    void onEnter(wxCommandEvent&);
    void onSpinChange(wxSpinDoubleEvent& event);
    void onModeChange(wxCommandEvent&);

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

    enum ModeChoice
    {
        FILL = 0,
        LINE = 1,
        POINT = 2
    };

    wxDECLARE_EVENT_TABLE();
};


struct MouseInfo
{
    bool spinning;
    bool moving;
    wxPoint mousePos;
    int wheelPos;
};

class Canvas : public wxGLCanvas
{
public:
    Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs);
    ~Canvas();

    bool wxGLCtxExists();
    bool graphicsManagerExists();
    void flip();
    float viewportAspectRatio();
    bool extCheck(std::pair<bool, std::string> in);
    std::shared_ptr<GraphicsManager> getGraphicsManager();
    MouseInfo getMouseInfo();
    void showErrorMessage(std::string title, std::string msg);

private:
    MainFrame* parentFrame;
    wxGLContext* wxGLCtx;
    std::shared_ptr<GraphicsManager> graphicsManager;
    bool done;
    bool debuggingExt;
    std::pair<int, int> viewportDims;
    bool cameraSpinning;
    bool cameraMoving;
    int mouseWheelPos;
    wxEvent* renderEvent;
    std::chrono::steady_clock::time_point lastFlip;
    float FPSSmoothing;
    float FPS;
    
    void onRender(wxCommandEvent&);
    void onClose(wxCloseEvent&);
    void onPaint(wxPaintEvent&);
    void onSize(wxSizeEvent&);
    void onLMBDown(wxMouseEvent&);
    void onLMBUp(wxMouseEvent&);
    void onRMBDown(wxMouseEvent&);
    void onRMBUp(wxMouseEvent&);
    void onWheel(wxMouseEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif /* MAIN_HPP_ */
