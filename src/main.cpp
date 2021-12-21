#include "main.hpp"


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(LOAD, MainFrame::load)
    EVT_MENU(wxID_ABOUT, MainFrame::about)
    EVT_MENU(wxID_EXIT, MainFrame::exit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    MainFrame *frame = new MainFrame("Hello World", 
                                     wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);

    if (!frame->opengl_initialized())
        return false;

    return true;
}

MainFrame::MainFrame(const wxString& title,
                     const wxPoint& pos, const wxSize& size)
         : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    #ifdef DEBUG
        logger_ptr = new wxLogStream(&std::cout);
        wxLog::SetActiveTarget(logger_ptr);
        wxLog::SetVerbose(true);
    #endif // DEBUG
    
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

    // SetMinSize(wxSize(250, 200));
}


bool MainFrame::opengl_initialized()
{
    if (canvas_ptr == NULL)
        return false;
    
    if (!canvas_ptr->glctx_exists())
        return false;
    
    return true;
}


void MainFrame::load(wxCommandEvent& event)
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


void MainFrame::about(wxCommandEvent& event)
{
    wxMessageBox("This is NOT a wxWidgets' Hello world sample",
                 "About Hello World", wxOK | wxICON_INFORMATION );
}


// https://github.com/wxWidgets/wxWidgets/tree/master/samples/opengl/pyramid
Canvas::Canvas(MainFrame* parent, const wxGLAttributes& canvasAttrs)
      : wxGLCanvas(parent, canvasAttrs), parent_ptr(parent)
{
    wxGLContextAttrs ctxAttrs;
    ctxAttrs.PlatformDefaults().OGLVersion(4, 7).EndList();
    glctx_ptr = new wxGLContext(this, NULL, &ctxAttrs);

    if ( !glctx_ptr->IsOK() )
    {
        wxMessageBox("The graphics driver failed to initialize OpenGL",
                     "OpenGL initialization error", wxOK | wxICON_ERROR, this);
        delete glctx_ptr;
        glctx_ptr = NULL;
    }
    else
    {
        wxLogVerbose("OpenGL successfully initialized");
    }
}
