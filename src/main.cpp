#include "main.hpp"
#include "graphics.hpp"

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    MainFrame *frame = new MainFrame("Hello World", 
                                     wxPoint(50, 50), wxSize(800, 600));
    frame->Show(true);

    if (!frame->opengl_initialized())
        return false;

    return true;
}


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(LOAD, MainFrame::onLoad)
    EVT_MENU(wxID_ABOUT, MainFrame::onAbout)
    EVT_MENU(wxID_EXIT, MainFrame::onExit)
wxEND_EVENT_TABLE()


MainFrame::MainFrame(const wxString& title,
                     const wxPoint& pos, const wxSize& size)
         : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    #ifdef DEBUG
        logger_ptr = new wxLogStream(&std::cout);
        wxLog::SetActiveTarget(logger_ptr);
        wxLog::SetVerbose(true);
    #endif /* DEBUG */
    
    wxMenu *menuContextFile = new wxMenu;
    menuContextFile->Append(Event::LOAD, "&Load...\tCtrl-L", "Load a OBJ file");
    menuContextFile->AppendSeparator();
    menuContextFile->Append(wxID_EXIT);

    wxMenu *menuContextHelp = new wxMenu;
    menuContextHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuContextFile, "&File");
    menuBar->Append(menuContextHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("No file loaded");

    wxGLAttributes glDefAttrs;
    glDefAttrs.PlatformDefaults().Defaults().EndList();
    bool supported = wxGLCanvas::IsDisplaySupported(glDefAttrs);

    if (supported)
    {
        wxLogVerbose("The display is supported with default attributes");
        canvas_ptr = new Canvas(this, glDefAttrs);
    }
    else
    {
        wxLogError("The display is not supported with default attributes");
        canvas_ptr = NULL;
    }

    SetMinSize(wxSize(250, 200));
}


bool MainFrame::opengl_initialized()
{
    if (canvas_ptr == NULL)
        return false;
    
    if (!canvas_ptr->glctx_exists())
        return false;
    
    return true;
}


void MainFrame::onLoad(wxCommandEvent&)
{
    wxFileDialog loadFileDialog(this, _("Load OBJ file"), "", "", 
                    "OBJ files (*.obj)|*.obj", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (loadFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxFileInputStream load_stream(loadFileDialog.GetPath());
    
    if (load_stream.IsOk())
    {
        wxLogVerbose("File opened: '%s'", loadFileDialog.GetPath());
        return;
    }
}


void MainFrame::onAbout(wxCommandEvent&)
{
    wxMessageBox("This is NOT a wxWidgets' Hello world sample",
                 "About Hello World", wxOK | wxICON_INFORMATION );
}


wxBEGIN_EVENT_TABLE(Canvas, wxGLCanvas)
    EVT_PAINT(Canvas::onPaint)
    EVT_SIZE(Canvas::onSize)
    // EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
wxEND_EVENT_TABLE()


// https://github.com/wxWidgets/wxWidgets/tree/master/samples/opengl/pyramid
Canvas::Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs)
      : wxGLCanvas(parent, canvasAttrs), parent_ptr(parent)
{
    wxGLContextAttrs ctxAttrs;
    ctxAttrs.PlatformDefaults().OGLVersion(OGL_MAJOR_VERSION,
                                            OGL_MINOR_VERSION).EndList();
    glctx_ptr = new wxGLContext(this, NULL, &ctxAttrs);

    if ( !glctx_ptr->IsOK() )
    {
        wxString msg_out;
        msg_out.Printf("The graphics driver failed to initialize OpenGL v%i.%i",
                            OGL_MAJOR_VERSION, OGL_MINOR_VERSION);
        wxMessageBox(msg_out,
                     "OpenGL initialization error", wxOK | wxICON_ERROR, this);
        delete glctx_ptr;
        glctx_ptr = NULL;
    }
    else
        wxLogVerbose("OpenGL v%i.%i successfully initialized",
                        OGL_MAJOR_VERSION, OGL_MINOR_VERSION);

    SetCurrent(*glctx_ptr);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        wxMessageBox("Glew failed to initialize", "Glew error",
                                    wxOK | wxICON_ERROR, this);
        delete glctx_ptr;
        glctx_ptr = NULL;
    }
    else
        wxLogVerbose("Glew successfully initialized");

    graphicsManager = std::make_unique<GraphicsManager>(this);

    // possible leak - don't know if wxWidgets automatically deletes this
    timer = new RenderTimer(this);
}


void Canvas::flip()
{
    SwapBuffers();
    graphicsManager->render();
}


void Canvas::onPaint(wxPaintEvent&)
{
    // this is mandatory to be able to draw in the window
    wxPaintDC dc(this);

    if (firstPaint)
    {
        flip();
        firstPaint = false;
    }
}


void Canvas::onSize(wxSizeEvent&)
{
    int usableWidth, usableHeight;
    GetClientSize(&usableWidth, &usableHeight);
    wxLogVerbose("Available window space: %ix%i", usableWidth, usableHeight);
    glViewport(0, 0, usableWidth, usableHeight);
}


void Canvas::log(std::string str)
{
    wxLogVerbose(str.c_str());
}
