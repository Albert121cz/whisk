#include "main.hpp"
#include "graphics.hpp"


wxIMPLEMENT_APP(App);

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

        SidePanel* side = new SidePanel(this, canvas->getGraphicsManager());
        mainSizer->Add(side, 0, wxEXPAND);
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


SidePanel::SidePanel(MainFrame* parent,
    std::shared_ptr<GraphicsManager> manager)
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    ObjectPanel* objects = new ObjectPanel(this, parent, manager);
    sizer->Add(objects, 1, wxEXPAND);

    ObjectSettings* settings = new ObjectSettings(this, objects->getListbox(),
        manager);
    sizer->Add(settings, 0, wxUP, 10);

    SetMaxSize(wxSize(280, -1));

    SetSizer(sizer);

    timer = new SidePanelRefreshTimer(manager, settings, objects->getListbox());
}


SidePanel::~SidePanel()
{
    delete timer;
}


wxBEGIN_EVENT_TABLE(ObjectPanel, wxPanel)
    EVT_CHECKLISTBOX(wxID_ANY, ObjectPanel::onCheckBox)
wxEND_EVENT_TABLE()


// https://zetcode.com/gui/wxwidgets/widgetsII/
ObjectPanel::ObjectPanel(SidePanel* parent, MainFrame* main, 
    std::shared_ptr<GraphicsManager> manager)
    : wxPanel(parent, wxID_ANY), graphicsManager(manager)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    listbox = new wxCheckListBox(this, wxID_ANY);
    sizer->Add(listbox, 4, wxEXPAND | wxALL, 5);

    buttons = new ObjectButtonPanel(graphicsManager, this, main, listbox);
    sizer->Add(buttons, 1, wxEXPAND | wxRIGHT, 5);

    SetSizer(sizer);
}


wxCheckListBox* ObjectPanel::getListbox()
{
    return listbox;
}



SidePanelRefreshTimer::SidePanelRefreshTimer(
    std::shared_ptr<GraphicsManager> manager, ObjectSettings* settings,
    wxCheckListBox* list)
    : graphicsManager(manager), objectSettings(settings), listbox(list)
{
    StartOnce(48);
}


wxDEFINE_EVENT(REFRESH_OBJECT_SETTINGS, wxCommandEvent);

void SidePanelRefreshTimer::Notify()
{
    std::vector<std::string> newNames = graphicsManager->getAllObjectNames();

    for (size_t i = 0; i < newNames.size(); i++)
    {
        if (i >= names.GetCount())
        {
            names.Add(newNames[i]);
            listbox->InsertItems(1, &names[i], i);
        }
        else if (newNames[i] != names[i])
        {
            names[i] = newNames[i];
            listbox->SetString(i, names[i]);
        }
            
        if (graphicsManager->getObjectShow(i) != listbox->IsChecked(i))
            listbox->Check(i, graphicsManager->getObjectShow(i));
    }

    for (size_t i = newNames.size(); i < names.GetCount(); i++)
    {
        names.RemoveAt(i, 1);
        listbox->Delete(i);
    }

    int selected = listbox->GetSelection();
    if (lastSelected != listbox->GetSelection())
    {
        wxEvent* event = new wxCommandEvent(REFRESH_OBJECT_SETTINGS);
        wxQueueEvent(objectSettings, event);
        lastSelected = selected;
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
    wxPanel* parentPanel, MainFrame* main, wxCheckListBox* target)
    : wxPanel(parentPanel, wxID_ANY), mainFrame(main), graphicsManager(manager)
{
    targetListbox = target;
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Objects");
    sizer->Add(label);

    wxButton* newButton = new wxButton(this, wxID_NEW, "New");
    sizer->Add(newButton, 0, wxTOP, 5);

    wxButton* renameButton = new wxButton(this, ID_RENAME, "Rename");
    sizer->Add(renameButton, 0, wxTOP, 5);

    wxButton* duplicateButton = new wxButton(this, ID_DUPLICATE, "Duplicate");
    sizer->Add(duplicateButton, 0, wxTOP, 5);
    
    wxButton* deleteButton = new wxButton(this, wxID_DELETE, "Delete");
    sizer->Add(deleteButton, 0, wxTOP, 5);

    SetSizer(sizer);
}


void ObjectButtonPanel::onNew(wxCommandEvent&)
{
    // TODO: interface with GraphicsManager
}


void ObjectButtonPanel::onRename(wxCommandEvent&)
{
    int idx = targetListbox->GetSelection();
    if (idx == wxNOT_FOUND)
        return;
    
    RenameFrame* frame = new RenameFrame(mainFrame, graphicsManager, idx);
    frame->Show();
    mainFrame->Disable();
}


void ObjectButtonPanel::onDuplicate(wxCommandEvent&)
{
    int idx = targetListbox->GetSelection();
    if (idx == wxNOT_FOUND)
        return;
    
    graphicsManager->duplicateObject(idx);
}


void ObjectButtonPanel::onDelete(wxCommandEvent&)
{
    int idx = targetListbox->GetSelection();
    if (idx == wxNOT_FOUND)
        return;

    graphicsManager->deleteObject(idx);
}


wxBEGIN_EVENT_TABLE(RenameFrame, wxFrame)
    EVT_TEXT_ENTER(wxID_ANY, RenameFrame::onEnter)
wxEND_EVENT_TABLE()


RenameFrame::RenameFrame(MainFrame* parent,
    std::shared_ptr<GraphicsManager> manager, int idx)
    : wxFrame(parent, wxID_ANY, "Rename", wxDefaultPosition, wxDefaultSize,
    wxCAPTION | wxFRAME_FLOAT_ON_PARENT), mainFrame(parent),
    graphicsManager(manager), objIdx(idx)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    textField = new wxTextCtrl(this, wxID_ANY, manager->getObjectName(idx),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(textField, 0, wxEXPAND);

    RenameFrameButtonPanel* buttons = new RenameFrameButtonPanel(this);
    sizer->Add(buttons, 0, wxTOP, 2);

    SetSizer(sizer);

    SetSize(sizer->ComputeFittingWindowSize(this));
    
    // calculate the position so it is in the middle of the main frame
    wxRect newRect = parent->GetRect().Deflate(
        (parent->GetSize().GetWidth() - GetSize().GetWidth())/2,
        (parent->GetSize().GetHeight() - GetSize().GetHeight())/2);
    SetPosition(newRect.GetPosition());
}


RenameFrame::~RenameFrame()
{
    mainFrame->Enable();
}


void RenameFrame::onEnter(wxCommandEvent&)
{
    graphicsManager->renameObject(objIdx, textField->GetValue().ToStdString());
    Close();
}


wxBEGIN_EVENT_TABLE(RenameFrameButtonPanel, wxPanel)
    EVT_BUTTON(wxID_OK, RenameFrameButtonPanel::onOk)
    EVT_BUTTON(wxID_CANCEL, RenameFrameButtonPanel::onCancel)
wxEND_EVENT_TABLE()


RenameFrameButtonPanel::RenameFrameButtonPanel(RenameFrame* parent)
    : wxPanel(parent), parentFrame(parent)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* okButton = new wxButton(this, wxID_OK, "Ok");
    sizer->Add(okButton, wxEXPAND);

    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    sizer->Add(cancelButton, wxEXPAND | wxLEFT, 5);

    SetSizer(sizer);
}


void RenameFrameButtonPanel::onOk(wxCommandEvent&)
{
    wxEvent* event = new wxCommandEvent(wxEVT_TEXT_ENTER);
    wxQueueEvent(parentFrame, event);
}


wxBEGIN_EVENT_TABLE(ObjectSettings, wxPanel)
    EVT_COMMAND(wxID_ANY, REFRESH_OBJECT_SETTINGS, ObjectSettings::onRefresh)
    EVT_TEXT_ENTER(wxID_ANY, ObjectSettings::onEnter)
    EVT_SPINCTRLDOUBLE(wxID_ANY, ObjectSettings::onChange)
wxEND_EVENT_TABLE()

ObjectSettings::ObjectSettings(wxPanel* parent, wxCheckListBox* list,
    std::shared_ptr<GraphicsManager> manager)
    : wxPanel(parent, wxID_ANY), listbox(list), graphicsManager(manager)
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(7, 10, 5);

    wxSize fieldSize = wxSize(61, -1);

    wxStaticText* positionText = new wxStaticText(this, wxID_ANY, "Pos.");
    sizer->Add(positionText, 0, wxLEFT, 3);

    wxStaticText* positionXText = new wxStaticText(this, wxID_ANY, "X:");
    textFields.push_back(new wxSpinCtrlDouble(this, POS_X, "",
        wxDefaultPosition, fieldSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER));
    sizer->Add(positionXText, 0, wxLEFT, 4);
    sizer->Add(textFields[POS_X]);

    wxStaticText* positionYText = new wxStaticText(this, wxID_ANY, "Y:");
    textFields.push_back(new wxSpinCtrlDouble(this, POS_Y, "",
        wxDefaultPosition, fieldSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER));
    sizer->Add(positionYText, 0, wxLEFT, 2);
    sizer->Add(textFields[POS_Y]);

    wxStaticText* positionZText = new wxStaticText(this, wxID_ANY, "Z:");
    textFields.push_back(new wxSpinCtrlDouble(this, POS_Z, "",
        wxDefaultPosition, fieldSize,  wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER));
    sizer->Add(positionZText, 0, wxLEFT, 2);
    sizer->Add(textFields[POS_Z]);

    for (int i = POS_X; i <= POS_Z; i++)
    {
        textFields[i]->SetRange(-99.99, 99.99);
        textFields[i]->SetIncrement(0.01);
        textFields[i]->SetDigits(2);
    }

    wxStaticText* rotationText = new wxStaticText(this, wxID_ANY, "Rot.");
    sizer->Add(rotationText, 0, wxLEFT, 3);

    wxStaticText* rotationXText = new wxStaticText(this, wxID_ANY, "X:");
    textFields.push_back(new wxSpinCtrlDouble(this, ROT_X, "",
        wxDefaultPosition, fieldSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER));
    sizer->Add(rotationXText, 0, wxLEFT, 4);
    sizer->Add(textFields[ROT_X]);

    wxStaticText* rotationYText = new wxStaticText(this, wxID_ANY, "Y:");
    textFields.push_back(new wxSpinCtrlDouble(this, ROT_Y, "", 
        wxDefaultPosition, fieldSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER));
    sizer->Add(rotationYText, 0, wxLEFT, 2);
    sizer->Add(textFields[ROT_Y]);

    wxStaticText* rotationZText = new wxStaticText(this, wxID_ANY, "Z:");
    textFields.push_back(new wxSpinCtrlDouble(this, ROT_Z, "",
        wxDefaultPosition, fieldSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER));
    sizer->Add(rotationZText, 0, wxLEFT, 2);
    sizer->Add(textFields[ROT_Z]);

    for (int i = ROT_X; i <= ROT_Z; i++)
    {
        textFields[i]->SetRange(0, 359.9);
        textFields[i]->SetIncrement(0.1);
        textFields[i]->SetDigits(1);
    }

    wxStaticText* sizeText = new wxStaticText(this, wxID_ANY, "Size");
    sizer->Add(sizeText, 0, wxLEFT, 3);

    textFields.push_back(new wxSpinCtrlDouble(this, SIZE, "", wxDefaultPosition,
        fieldSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER));
    sizer->AddStretchSpacer();
    sizer->Add(textFields[SIZE]);

    textFields[SIZE]->SetRange(0, 99.999);
    textFields[SIZE]->SetIncrement(0.01);
    textFields[SIZE]->SetDigits(3);

    SetSizer(sizer);
};


void ObjectSettings::onRefresh(wxCommandEvent&)
{
    int selected = listbox->GetSelection();
    
    if (selected == wxNOT_FOUND)
    {
        for (size_t i = 0; i < textFields.size(); i++)
            textFields[i]->SetValue(0);
        return;
    }

    glm::vec3* pos = graphicsManager->getObjectPosVec(selected);
    glm::vec3* rot = graphicsManager->getObjectRotVec(selected);
    glm::vec3* size = graphicsManager->getObjectSize(selected);

    float values[] = {(*pos).x, (*pos).y, (*pos).z,
        (*rot).x, (*rot).y, (*rot).z, (*size).x};

    for (size_t i = 0; i < textFields.size(); i++)
        textFields[i]->SetValue(wxString::Format("%f", values[i]));
}


void ObjectSettings::onEnter(wxCommandEvent&)
{
    // kick the user out of the field, so the changes take place
    Navigate();
}


void ObjectSettings::onChange(wxSpinDoubleEvent& event)
{
    int fieldID = event.GetId();

    if (listbox->GetSelection() == wxNOT_FOUND)
    {
        textFields[fieldID]->SetValue(0);
        return;
    }

    int selected = listbox->GetSelection();

    glm::vec3* pos = graphicsManager->getObjectPosVec(selected);
    glm::vec3* rot = graphicsManager->getObjectRotVec(selected);
    glm::vec3* size = graphicsManager->getObjectSize(selected);

    float* values[] = {&pos->x, &pos->y, &pos->z,  &rot->x, &rot->y, &rot->z,
        &size->x, &size->y, &size->z};
    
    float fieldValue = textFields[fieldID]->GetValue();
    
    *values[fieldID] = fieldValue;

    // size has 3 dimensions and user edits them all at once
    if (fieldID == SIZE)
    {
        *values[SIZE + 1] = fieldValue;
        *values[SIZE + 2] = fieldValue;
    }
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
