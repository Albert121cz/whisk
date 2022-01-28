#include "main.hpp"
#include "graphics.hpp"

wxIMPLEMENT_APP(App);

// TODO: make wxListBox with objects and textures in different frame
// -> https://zetcode.com/gui/wxwidgets/widgetsII/


bool App::OnInit()
{
    wxInitAllImageHandlers();

    frame = new MainFrame("Hello World", 
                                     wxPoint(50, 50), wxSize(800, 600));
    frame->Show(true);

    if (!frame->openGLInitialized())
        return false;

    return true;
}


int App::OnExit()
{
    delete frame;
    return 0;
}


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(LOAD_OBJ, MainFrame::onObjLoad)
    EVT_MENU(LOAD_TEX, MainFrame::onTexLoad)
    EVT_MENU(wxID_ABOUT, MainFrame::onAbout)
    EVT_MENU(wxID_EXIT, MainFrame::onExit)
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
    
    wxMenu *menuContextFile = new wxMenu;
    menuContextFile->Append(Event::LOAD_OBJ, "&Load object...\tCtrl-O",
        "Load OBJ file");
    menuContextFile->Append(Event::LOAD_TEX, "&Load texture...\tCtrl-T",
        "Load texture file");
    menuContextFile->AppendSeparator();
    menuContextFile->Append(wxID_EXIT);

    wxMenu *menuContextHelp = new wxMenu;
    menuContextHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
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

        timer = new RenderTimer(this, 1, canvas);
        
    }
    else
    {
        wxLogError("The display is not supported with default attributes");
        canvas = NULL;
    }

    SetMinSize(wxSize(250, 200));
}


MainFrame::~MainFrame()
{
    delete timer;
}


bool MainFrame::openGLInitialized()
{
    if (canvas == NULL)
        return false;
    
    if (!canvas->wxGLCtxExists())
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
                 "About Hello World", wxOK | wxICON_INFORMATION );
}


void MainFrame::onExit(wxCommandEvent&)
{
    timer->Stop(); 
    Close(true);
}


wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
    EVT_PAINT(Canvas::onPaint)
    EVT_SIZE(Canvas::onSize)
    EVT_ENTER_WINDOW(Canvas::onEnteringWindow)
    EVT_LEAVE_WINDOW(Canvas::onLeavingWindow)
    EVT_RIGHT_DOWN(Canvas::onRMBDown)
    EVT_RIGHT_UP(Canvas::onRMBUp)
wxEND_EVENT_TABLE()


// https://github.com/wxWidgets/wxWidgets/tree/master/samples/opengl/pyramid
Canvas::Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs)
      : wxGLCanvas(parent, canvasAttrs), parent_ptr(parent)
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

    if (GLEW_KHR_debug)
    {
        debuggingExt = true;
        wxLogVerbose("Extension KHR_debug supported");
    }
    else
        wxLogVerbose("Extension KHR_debug not supported, GL messages disabled");

    if (GLEW_ARB_direct_state_access)
        wxLogVerbose("Extension ARB_direct_state_access supported");
    else
    {
        wxMessageBox("Your driver does not support ARB_direct_state_access",
            "Initialization error", wxOK | wxICON_ERROR, this);
        return;
    }

    graphicsManager = std::make_unique<GraphicsManager>(this);
}


void Canvas::flip()
{
    wxClientDC dc(this);

    SwapBuffers();
    graphicsManager->render();
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


// BUG: it is possible to leave the window with the camera still moving
void Canvas::onRMBDown(wxMouseEvent&)
{
    if (!mouseInsideWindow)
        return;

    cameraMoving = true;
    mousePos = wxGetMousePosition();
}
