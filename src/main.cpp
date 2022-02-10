#include "main.hpp"
#include "graphics.hpp"

wxIMPLEMENT_APP(App);

// TODO: make wxListBox with objects and textures in different frame
// these can be aligned using wxLayoutAlgorithm
// -> https://zetcode.com/gui/wxwidgets/widgetsII/


bool App::OnInit()
{
    wxInitAllImageHandlers();

    frame = new MainFrame("Hello World", wxPoint(50, 50), wxSize(800, 600));

    if (!frame->openGLInitialized())
        return false;    

    frame->Show(true);
    return true;
}


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(LOAD_OBJ, MainFrame::onObjLoad)
    EVT_MENU(LOAD_TEX, MainFrame::onTexLoad)
    EVT_MENU(wxID_ABOUT, MainFrame::onAbout)
    EVT_MENU(wxID_EXIT, MainFrame::onExit)
    EVT_CLOSE(MainFrame::onClose)
wxEND_EVENT_TABLE()


MainFrame::MainFrame(const wxString& title,
                     const wxPoint& pos, const wxSize& size)
         : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    #ifdef DEBUG
        logger = new wxLogStream(&std::cout);
        wxLog::SetActiveTarget(logger);
        wxLog::SetVerbose(true);
    #endif /* DEBUG */

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxMenu* menuContextFile = new wxMenu;
    menuContextFile->Append(Event::LOAD_OBJ, "&Load object...\tCtrl-O",
        "Load OBJ file");
    menuContextFile->Append(Event::LOAD_TEX, "&Load texture...\tCtrl-T",
        "Load texture file");
    menuContextFile->AppendSeparator();
    menuContextFile->Append(wxID_EXIT);

    wxMenu* menuContextHelp = new wxMenu;
    menuContextHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuContextFile, "&File");
    menuBar->Append(menuContextHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar(2);
    SetStatusText("No file loaded");

    wxGLAttributes glDefAttrs;
    glDefAttrs.PlatformDefaults().Defaults().EndList();
    bool supported = wxGLCanvas::IsDisplaySupported(glDefAttrs);

    if (supported)
    {
        wxLogVerbose("The display is supported with default attributes");

        canvas = new Canvas(this, glDefAttrs);
        mainSizer->Add(canvas, 1, wxEXPAND);

        objects = new ObjectPanel(this, canvas->getGraphicsManager());
        mainSizer->Add(objects, 0, wxEXPAND);
    }
    else
    {
        wxLogError("The display is not supported with default attributes");
        canvas = NULL;
    }

    SetMinSize(wxSize(800, 600));
    SetSizer(mainSizer);
}


bool MainFrame::openGLInitialized()
{
    if (canvas == NULL)
        return false;
    
    if (!canvas->wxGLCtxExists())
        return false;

    if (!canvas->graphicsManagerExists())
        return false;
    
    return true;
}


void MainFrame::onObjLoad(wxCommandEvent&)
{
    wxFileDialog loadFileDialog(this, _("Load OBJ file"), "", "", 
                    "OBJ (*.obj)|*.obj", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (loadFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileInputStream load_stream(loadFileDialog.GetPath());
    
    if (load_stream.IsOk())
    {
        wxLogVerbose("Object file opened: '%s'", loadFileDialog.GetPath());
        return;
    }
}


void MainFrame::onTexLoad(wxCommandEvent&)
{
    wxFileDialog loadFileDialog(this, _("Load texture file"), "", "", 
    "PNG (*.png)|*.png|BPM (*.bmp)|*.bmp|JPEG (*.jpg; *.jpeg)|*.jpg;*.jpeg", 
                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (loadFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxImage image(loadFileDialog.GetPath());


    if (image.IsOk())
    {
        wxLogVerbose("Texture file opened: '%s'", loadFileDialog.GetPath());
        // mirror vertically
        image = image.Mirror(false);
        canvas->addTex(image.GetData(), image.GetWidth(), image.GetHeight());
    }
    else
        wxMessageBox("The texture file is not a valid image file",
                "Texture load error", wxOK | wxICON_ERROR, this);
}


void MainFrame::onAbout(wxCommandEvent&)
{
    wxMessageBox("This is NOT a wxWidgets' Hello world sample",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}


void MainFrame::onExit(wxCommandEvent&)
{
    Close();
}


// https://docs.wxwidgets.org/3.0/classwx_close_event.html
void MainFrame::onClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {

        if (wxMessageBox("Do you wish to close the app?", "Quit confirmation",
            wxICON_QUESTION | wxYES_NO) == wxYES)
            Destroy();
        else
            event.Veto();
    }
}


wxBEGIN_EVENT_TABLE(ObjectPanel, wxPanel)
    EVT_CHECKLISTBOX(wxID_ANY, ObjectPanel::onCheckBox)
wxEND_EVENT_TABLE()


// https://zetcode.com/gui/wxwidgets/widgetsII/
ObjectPanel::ObjectPanel(MainFrame* parent, 
    std::shared_ptr<GraphicsManager> manager)
    : wxPanel(parent, wxID_ANY), graphicsManager(manager)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    listbox = new wxCheckListBox(this, wxID_ANY);
    sizer->Add(listbox, 4, wxEXPAND | wxALL, 5);

    buttons = new ObjectButtonPanel(graphicsManager, this, listbox);
    sizer->Add(buttons, 1, wxEXPAND | wxRIGHT, 5);

    SetMaxSize(wxSize(250, -1));
    SetSizer(sizer);

    timer = new ListRefreshTimer(graphicsManager, listbox);
}


ObjectPanel::~ObjectPanel()
{
    delete timer;
}


ListRefreshTimer::ListRefreshTimer(std::shared_ptr<GraphicsManager> manager,
    wxCheckListBox* list)
    : graphicsManager(manager), listbox(list)
{
    StartOnce(250);
}


void ListRefreshTimer::Notify()
{
    std::vector<std::string> newNames = graphicsManager->getObjectNames();

    for (size_t i = 0; i < newNames.size(); i++)
    {
        if (i >= names.GetCount() || newNames[i] != names[i])
        {
            names.Insert(newNames[i], i);
            listbox->InsertItems(1, &names[i], i);
        }

        if (graphicsManager->getObjectShow(i) && !listbox->IsChecked(i))
            listbox->Check(i);
    }
    
    StartOnce();
}


void ObjectPanel::onCheckBox(wxCommandEvent& event)
{
    int itemIdx = event.GetInt();
    graphicsManager->showOrHideObject(itemIdx);
}


wxBEGIN_EVENT_TABLE(ObjectButtonPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, ObjectButtonPanel::onNew)
    EVT_BUTTON(ID_RENAME, ObjectButtonPanel::onRename)
    EVT_BUTTON(ID_DUPLICATE, ObjectButtonPanel::onDuplicate)
    EVT_BUTTON(wxID_DELETE, ObjectButtonPanel::onDelete)
wxEND_EVENT_TABLE()


ObjectButtonPanel::ObjectButtonPanel(std::shared_ptr<GraphicsManager> manager,
    wxPanel* parentPanel, wxCheckListBox* target)
    : wxPanel(parentPanel, wxID_ANY), graphicsManager(manager)
{
    targetListbox = target;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticText* label = new wxStaticText(this, wxID_ANY, wxT("Objects"));
    sizer->Add(label);

    newButton = new wxButton(this, wxID_NEW, wxT("New"));
    sizer->Add(newButton, 0, wxTOP, 5);

    renameButton = new wxButton(this, ID_RENAME, wxT("Rename"));
    sizer->Add(renameButton, 0, wxTOP, 5);

    duplicateButton = new wxButton(this, ID_DUPLICATE, wxT("Duplicate"));
    sizer->Add(duplicateButton, 0, wxTOP, 5);
    
    deleteButton = new wxButton(this, wxID_DELETE, wxT("Delete"));
    sizer->Add(deleteButton, 0, wxTOP, 5);

    SetSizer(sizer);
}


void ObjectButtonPanel::onNew(wxCommandEvent&)
{
    // TODO: interface with GraphicsManager
}


void ObjectButtonPanel::onRename(wxCommandEvent&)
{
    // TODO: interface with GraphicsManager
}


void ObjectButtonPanel::onDuplicate(wxCommandEvent&)
{
    int idx = targetListbox->GetSelection();
    if (idx != wxNOT_FOUND)
        graphicsManager->duplicateObject(idx);
}


void ObjectButtonPanel::onDelete(wxCommandEvent&)
{
    // TODO: interface with GraphicsManager
}


wxDEFINE_EVENT(RENDER, wxCommandEvent);

wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
    EVT_COMMAND(wxID_ANY, RENDER, Canvas::onRender)
    EVT_CLOSE(Canvas::onClose)
    EVT_PAINT(Canvas::onPaint)
    EVT_SIZE(Canvas::onSize)
    EVT_LEAVE_WINDOW(Canvas::onLeavingWindow)
    EVT_RIGHT_DOWN(Canvas::onRMBDown)
    EVT_RIGHT_UP(Canvas::onRMBUp)
wxEND_EVENT_TABLE()


// https://github.com/wxWidgets/wxWidgets/tree/master/samples/opengl/pyramid
Canvas::Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs)
      : wxGLCanvas(parent, canvasAttrs), parentFrame(parent)
{
    wxGLContextAttrs ctxAttrs;
    ctxAttrs.PlatformDefaults().OGLVersion(OGL_MAJOR_VERSION,
                                            OGL_MINOR_VERSION).EndList();
    wxGLCtx = new wxGLContext(this, NULL, &ctxAttrs);

    if ( !wxGLCtx->IsOK() )
    {
        wxString msg_out;
        msg_out.Printf("The graphics driver failed to initialize OpenGL v%i.%i",
                            OGL_MAJOR_VERSION, OGL_MINOR_VERSION);
        wxMessageBox(msg_out,
                     "OpenGL initialization error", wxOK | wxICON_ERROR, this);
        delete wxGLCtx;
        wxGLCtx = nullptr;
        return;
    }
    wxLogVerbose("OpenGL v%i.%i successfully initialized",
        OGL_MAJOR_VERSION, OGL_MINOR_VERSION);

    SetCurrent(*wxGLCtx);

    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        wxLogVerbose("%s", glewGetErrorString(error));
        wxMessageBox("Glew failed to initialize", "Glew error",
            wxOK | wxICON_ERROR, this);
        delete wxGLCtx;
        wxGLCtx = nullptr;
        return;
    }
    wxLogVerbose("Glew successfully initialized");

    std::vector<std::pair<bool, std::string>> extensions = 
    {
        #ifdef DEBUG
            {GLEW_KHR_debug, "KHR_debug"},
        #endif /* DEBUG */
        {GLEW_ARB_direct_state_access, "ARB_direct_state_access"},
        {GLEW_ARB_bindless_texture, "ARB_bindless_texture"},
        {WGLEW_EXT_swap_control, "EXT_swap_control"},
    };

    for (auto extension : extensions)
    {
        if (!extCheck(extension))
            return;
    }

    graphicsManager = std::make_shared<GraphicsManager>(this);

    renderEvent = new wxCommandEvent(RENDER);
    lastFlip = std::chrono::steady_clock::now();
    wxQueueEvent(this, renderEvent);
}


void Canvas::flip()
{
    wxClientDC dc(this);

    graphicsManager->render();
    SwapBuffers();
}


void Canvas::addTex(const unsigned char* data, int width, int height)
{
    graphicsManager->getTexManagerPtr()->addTexture(data, width, height);
}


float Canvas::viewportAspectRatio()
{
    float width = static_cast<float>(viewportDims.first);
    float height = static_cast<float>(viewportDims.second);
    return width / height;
}


void Canvas::log(std::string str)
{
    wxLogVerbose(str.c_str());
}


bool Canvas::extCheck(std::pair<bool, std::string> in)
{
    if (in.first)
    {
        wxLogVerbose("Extension %s supported", in.second);
        return true;
    }
    else
    {
        wxString msg;
        msg.Printf("The GPU driver does not support %s extension", in.second);
        wxMessageBox(msg, "Initialization error", wxOK | wxICON_ERROR, this);
        return false;
    }
}


// https://stackoverflow.com/a/87333
// https://stackoverflow.com/a/27739925
void Canvas::onRender(wxCommandEvent&)
{
    flip();

    std::chrono::steady_clock::time_point currentFlip;
    currentFlip = std::chrono::steady_clock::now();
    float difference = std::chrono::duration_cast<std::chrono::microseconds>
        (currentFlip - lastFlip).count();
    FPS = (FPS * FPSSmoothing) + (1000000/difference * (1.0-FPSSmoothing));
    lastFlip = currentFlip;
    parentFrame->SetStatusText(wxString::Format(wxT("%.1f FPS"), FPS), 1);

    // give control back to the app to handle all UI elements and events
    wxGetApp().Yield();

    if (done)
        return;

    renderEvent = new wxCommandEvent(RENDER);
    wxQueueEvent(this, renderEvent);
}


void Canvas::onClose(wxCloseEvent&)
{
    done = true;
}


void Canvas::onPaint(wxPaintEvent&)
{
    // this is mandatory to be able to draw in the window
    wxPaintDC dc(this);

    graphicsManager->render();
    SwapBuffers();
}


void Canvas::onSize(wxSizeEvent&)
{

    GetClientSize(&viewportDims.first, &viewportDims.second);
    wxLogVerbose("Viewport dimensions: %ix%i", 
        viewportDims.first, viewportDims.second);
    glViewport(0, 0, viewportDims.first, viewportDims.second);
}


void Canvas::onRMBDown(wxMouseEvent&)
{
    cameraMoving = true;
    mousePos = wxGetMousePosition();
}
