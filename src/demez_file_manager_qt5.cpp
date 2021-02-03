#include "demez_file_manager_qt5.h"
#include "platform.h"
#include "util.h"
#include "util_qt.h"


// https://forum.qt.io/topic/101391/windows-10-dark-theme/4

FileManagerUI* g_window;
DirectoryLoader* g_dirLoader;


void SetToDetailView()
{
    g_window->SetViewMode(EFolderViewMode::DETAIL);
}

void SetToIconView()
{
    g_window->SetViewMode(EFolderViewMode::ICON);
}


// ================================================================


FileManagerUI::FileManagerUI(QWidget *parent):
    QWidget(parent)
{
    m_places = new Places;
    m_mainLayout = new QVBoxLayout;
    m_navBarLayout = new QHBoxLayout;
    m_toolBarLayout = new QHBoxLayout;
    m_splitLayout = new QHBoxLayout;
    m_fileViewLayout = new QVBoxLayout;
    m_directory = new DirectoryBar;

    setLayout(m_mainLayout);

    // m_mainLayout->setContentsMargins(0, 0, 0, 0);

    NavButton* btnPrev = new NavButton("<", this);
    NavButton* btnForward = new NavButton(">", this);
    NavButton* btnUp = new NavButton("/\\", this);
    NavButton* btnGo = new NavButton("->", this);

    connect(btnUp, &NavButton::pressed, this, &FileManagerUI::BtnNavUp);
    connect(btnGo, &NavButton::pressed, this, &FileManagerUI::BtnNavGo);

    QPushButton* btnViewDetail = new QPushButton("Detail View", this);
    QPushButton* btnViewIcon = new QPushButton("Icon View", this);

    connect(btnViewDetail, &QPushButton::pressed, &SetToDetailView);
    connect(btnViewIcon, &QPushButton::pressed, &SetToIconView);

    m_navBarLayout->addWidget(btnPrev);
    m_navBarLayout->addWidget(btnForward);
    m_navBarLayout->addWidget(btnUp);
    m_navBarLayout->addWidget(m_directory);
    m_navBarLayout->addWidget(btnGo);

    m_toolBarLayout->addWidget(btnViewDetail);
    m_toolBarLayout->addWidget(btnViewIcon);

    m_mainLayout->addLayout(m_navBarLayout);
    m_mainLayout->addLayout(m_toolBarLayout);
    m_mainLayout->addLayout(m_splitLayout);

    m_splitLayout->addWidget(m_places);

    m_folderView = nullptr;
    SetViewMode(EFolderViewMode::ICON);
    // SetViewMode(EFolderViewMode::DETAIL);

    m_viewManager = GetIFolderViewManager();
    m_cxtMenuHandler = GetIContextMenuHandler();

    m_cxtMenuThread = new QThread;

    m_cxtMenuHandler->moveToThread(m_cxtMenuThread);

    // connect(this, &FileManagerUI::CxtTest, m_cxtMenuHandler, &IContextMenuHandler::Test);

    // issue with this: where am i getting the values for calling the Display Function?
    // actually, i can just pass everything into LoadMenu, and then that will pass it all into LoadingFinished
    // which will then call DisplayMenu, just in case it needs to be displayed on the main thread

    // hopefully it will just call this on the main thread, idk
    connect(m_cxtMenuHandler, &IContextMenuHandler::LoadingFinished, this, &FileManagerUI::DisplayContextMenu);
    // connect(m_cxtMenuHandler, &IContextMenuHandler::LoadingFinished, m_cxtMenuHandler, &IContextMenuHandler::Test);

    // can't queue std::vector<fs::path> ????
    qRegisterMetaType< std::vector<fs::path> >( "std::vector<fs::path>" );
    connect(this, &FileManagerUI::CxtLoadMenu, m_cxtMenuHandler, &IContextMenuHandler::LoadMenu);

   //  connect(this, &FileManagerUI::CxtTest, m_cxtMenuHandler, &IContextMenuHandler::Test);

    m_cxtMenuThread->start();

    //
}

#define DELETE(var) if (var != nullptr) delete var; var = nullptr

FileManagerUI::~FileManagerUI()
{
    DELETE(m_places);
    DELETE(m_mainLayout);
    DELETE(m_navBarLayout);
    DELETE(m_toolBarLayout);
    DELETE(m_splitLayout);
    DELETE(m_fileViewLayout);
    DELETE(m_cxtMenuThread);
    DELETE(m_cxtMenuHandler);
    // DELETE(m_viewManager);
    DELETE(m_folderView);
}

#undef DELETE


void FileManagerUI::BtnNavUp()
{
    fs::path path = QStrToWStr(m_directory->GetPath());
    if (path.has_parent_path() && path.parent_path().wstring() != m_currentDir)
    {
        LoadDirectory(path.parent_path().wstring());
    }
}

void FileManagerUI::BtnNavGo()
{
    std::wstring pathStr = QStrToWStr(m_directory->GetPath());
    // if (pathStr != m_currentDir)
    {
        LoadDirectory(pathStr);
    }
}


void FileManagerUI::LoadDirectory(const std::wstring& path)
{
    if (!fs::is_directory(path))
    {
        return;
    }

    g_dirLoader->LoadDirectory(path);

    try
    {
        // FolderViewSettings settings;
        // m_viewManager->GetViewSettings(m_currentDir.c_str(), settings);
        // SetViewMode(settings.viewMode);

        m_folderView->DisplayDirectory(path);
        m_currentDir = path;
        m_directory->SetPath(path.c_str());
    }
    catch (fs::filesystem_error)
    {
        printf("Error loading directory \"%ls\"", path.c_str());
    }
}


void FileManagerUI::SetViewMode(EFolderViewMode fileView)
{
    if (fileView == m_viewMode)
    {
        return;
    }

    if (m_folderView != nullptr)
    {
        m_splitLayout->removeWidget(m_folderView->GetWidget());
        delete m_folderView;
    }

    switch(fileView)
    {
        case EFolderViewMode::ICON:
            m_folderView = new FileListIconView;
            break;

        case EFolderViewMode::DETAIL:
        default:
            m_folderView = new FileListDetailView;
            break;
    }

    m_splitLayout->addWidget(m_folderView->GetWidget());
    m_viewMode = fileView;

    LoadDirectory(QStrToWStr(m_directory->GetPath()));
}


void FileManagerUI::OpenContextMenu(QPoint pos, std::vector<fs::path> items)
{
    m_cxtMenuHandler->LoadMenu(pos, m_currentDir.c_str(), items);
    m_cxtMenuHandler->DisplayMenu(pos, m_currentDir.c_str(), items);

    // emit CxtLoadMenu(pos, m_currentDir.c_str(), items);
    // emit CxtDisplayMenu(pos, m_currentDir.c_str(), items);
    // std::vector<const char*> tmp;

    // emit CxtTest(tmp);

    // idk where im going to delete this lol
    // i can't connect a selected function due to the OS function connecting them
    // unless i go with connecting it to the ui first, and then calling the OS to run the action
    // then i can delete it easily
    /*if (m_cxtMenu != nullptr)
    {
        delete m_cxtMenu;
    }

    m_cxtMenu = new QMenu;

    // Load context menu items
    OS_LoadContextMenu(m_cxtMenu, m_currentDir.c_str(), items);*/

    /*for (QAction* action: m_cxtMenu->actions())
    {
        // TEST THIS
        connect(action, &QAction::triggered, this, &FileManagerUI::RunContextMenuAction);
    }*/

    // m_cxtMenu->popup(pos);
}


void FileManagerUI::DisplayContextMenu(QPoint pos, std::vector<fs::path> items)
{
    // Stay on this thread to draw it
    m_cxtMenuHandler->DisplayMenu(pos, m_currentDir.c_str(), items);

    // emit CxtLoadMenu(m_currentDir.c_str(), items);
    // emit CxtDisplayMenu(pos, m_currentDir.c_str(), items);
}


void FileManagerUI::RunContextMenuAction()
{
    printf("yoooo\n");

    // OS_RunContextMenuItem(action, m_currentDir.c_str(), m_folderView->GetSelectedFiles());
}


void FileManagerUI::HandleFocusChange(QWidget *old, QWidget *now)
{
    if (old == (QWidget*)m_directory || now == (QWidget*)m_directory)
    {
        printf("directory bar\n");
    }

    printf("yoooo\n");
}


// ================================================================


DirectoryBarEdit::DirectoryBarEdit(DirectoryBar* dirBar):
    QLineEdit()
{
    m_dirBar = dirBar;
    setFixedHeight(24);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    // setText(ToQString(m_dirBar->m_path));
}

DirectoryBarEdit::~DirectoryBarEdit()
{

}


void DirectoryBarEdit::keyPressEvent(QKeyEvent* e)
{
    int key = e->key();
    if (key == Qt::Key::Key_Return)
    {
        g_window->BtnNavGo();
    }
    else
    {
        QLineEdit::keyPressEvent(e);
    }
}


void DirectoryBarEdit::focusOutEvent(QFocusEvent* e)
{
    m_dirBar->HidePathEdit();
}


// ================================================================


DirectoryBar::DirectoryBar(QWidget* parent):
    QWidget(parent)
{
    m_layout = new QHBoxLayout;
    m_lineLayout = new QHBoxLayout;

    m_lineEdit = new DirectoryBarEdit(this);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    setLayout(m_layout);
    // setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setFixedHeight(24);

    setStyleSheet("border: 2px solid; border-color: #660000;");
}

DirectoryBar::~DirectoryBar()
{
}


void DirectoryBar::keyPressEvent(QKeyEvent* e)
{
    int key = e->key();
    if (key == Qt::Key::Key_Return)
    {
        g_window->BtnNavGo();
    }
    else
    {
        // QLineEdit::keyPressEvent(e);
        // m_lineEdit->keyPressEvent(e);
    }
}


void DirectoryBar::mousePressEvent(QMouseEvent* e)
{
    // hide the path buttons, and start text editing
    // but then how do we know to show the buttons again?
    // are we able to check if focus is lost?

    printf("dir bar mouse press\n");
    ShowPathEdit();
}


QString DirectoryBar::GetPath()
{
    // if (m_lineEdit != nullptr)
    if (!m_lineEdit->isHidden())
    {
        return m_lineEdit->text();
    }
    else
    {
        return ToQString(m_path);
    }
}


void DirectoryBar::SetPath(const fs::path& path)
{
    // m_lineEdit->setText(ToQString(path));
    m_path = path;

    // blech
    ClearLayout(m_layout);
    m_layout->addStretch();

    std::vector<std::string> folders;

    fs::path part = path;

    fs::path tmp = part.parent_path();

    // if (part.has_parent_path())
    // if (part.root_directory().string() != part.string())
    // if (PathToCStr(part.root_directory()) != PathToCStr(part.string()))

    while (part.parent_path().compare(part) != 0)
    {
        AddPathPartButton(part);
        part = part.parent_path();
    }

    // add the root path
    AddPathPartButton(part);
}


void DirectoryBar::AddPathPartButton(fs::path& path)
{
    QString partStr = QString::fromWCharArray(path.c_str());

    DirectoryBarButton* partBtn = new DirectoryBarButton(path);
    // partBtn->resize(partBtn->sizeHint().width(), partBtn->sizeHint().height());
    // partBtn->setFlat(true);
    // partBtn->setMinimumWidth(16);
    // partBtn->setText(partStr);
    // partBtn->setFixedSize(partBtn->sizeHint().width(), partBtn->sizeHint().height());
    // partBtn->setFixedSize(partBtn->minimumSizeHint().width(), partBtn->minimumSizeHint().height());

    m_layout->insertWidget(0, partBtn);
    // m_layout->addWidget(partBtn);
}


void DirectoryBar::ShowPathEdit()
{
    ClearLayout(m_layout);
    m_layout->addWidget(m_lineEdit);
    m_lineEdit->setText(ToQString(m_path));
    m_lineEdit->show();
    m_lineEdit->setFocus();
}

void DirectoryBar::HidePathEdit()
{
    m_lineEdit->hide();
    m_layout->removeWidget(m_lineEdit);

    // delete m_lineEdit;
    SetPath(m_path);
    // m_lineEdit = nullptr;
}


// ================================================================


// blech
#define COLOR_NONE "background: transparent;"
#define COLOR_HOVER "background: #a1dcff;"


DirectoryBarButton::DirectoryBarButton(fs::path& part)
{
    // m_label = new QLabel;
    SetPath(part);

    // setFixedSize(minimumSizeHint().width(), minimumSizeHint().height());
    setFixedWidth(minimumSizeHint().width() + 8);
    setAlignment(Qt::AlignCenter);
}

DirectoryBarButton::~DirectoryBarButton()
{
}


void DirectoryBarButton::SetPath(fs::path part)
{
    fs::path partName = part;

    if (part.has_filename())
    {
        partName = part.filename();
    }

    QString partStr = QString::fromWCharArray(partName.c_str());
    setText(partStr);
    m_path = part;
}

// hover events
void DirectoryBarButton::enterEvent(QEvent* e)
{
    setStyleSheet(COLOR_HOVER);
}

void DirectoryBarButton::leaveEvent(QEvent* e)
{
    setStyleSheet(COLOR_NONE);
}

void DirectoryBarButton::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        g_window->LoadDirectory(m_path);
    }
}


// ================================================================


Places::Places(QWidget* parent):
    QWidget(parent)
{
    // temporary
    setMaximumWidth(128);

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);

    m_bookmarks = new QListWidget;
    m_drives = new QListWidget;

    QLabel* labelBookmarks = new QLabel("Bookmarks");
    QLabel* labelDrives = new QLabel("Drives");

    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(labelBookmarks);
    layout->addWidget(m_bookmarks);

    layout->addWidget(labelDrives);
    layout->addWidget(m_drives);

    LoadPlaces();
}

Places::~Places()
{
}


void Places::LoadPlaces()
{
    LoadBookmarks();
    LoadDrives();
}


void Places::LoadBookmarks()
{
    m_bookmarkPaths = OS_GetBookmarks();

    for (fs::path bookmark: m_bookmarkPaths)
    {
        m_bookmarks->addItem( ToQString(bookmark.filename()) );
    }
}


void Places::LoadDrives()
{
    for (std::wstring drive: OS_GetMountedDrives())
    {
        m_drives->addItem(ToQString(drive));
    }
}


// ================================================================
// TODO: move this elsewhere, not really qt related, until i make this a thread i guess

DirectoryLoader::DirectoryLoader()
{
}


DirectoryLoader::~DirectoryLoader()
{
}


// TODO: have this be on another thread, and use a callback or signal/slot to add files to the directory viewer
void DirectoryLoader::LoadDirectory(const fs::path& path)
{
    m_folders.clear();
    m_files.clear();
    bool isDir = false;

    for (fs::path file: fs::directory_iterator(path))
    {
        isDir = false;

        try
        {
            isDir = fs::is_directory(file);
        }
        catch (fs::filesystem_error)
        {
            isDir = false;
        }

        if (isDir)
        {
            m_folders.push_back(file);
        }
        else
        {
            m_files.push_back(file);
        }

        // emit FileAddedCallback(file);
    }
}


// ================================================================


NavButton::NavButton(const char* name, QWidget* parent):
    QPushButton(name, parent)
{
    setFixedSize(24, 24);
}

NavButton::~NavButton()
{
}


// ================================================================


auto main(int argc, char* argv[]) -> int
{
    QApplication app(argc, argv);
    // QGuiApplication appGUI(argc, argv);
    app.setDesktopSettingsAware(true);

    int init = OS_Init();
    if (init != 0)
    {
        return init;
    }

    g_dirLoader = new DirectoryLoader;
    g_window = new FileManagerUI;

    if (argc == 1)
    {

    }

    QObject::connect(&app, &QApplication::focusChanged, g_window, &FileManagerUI::HandleFocusChange);

    g_window->resize(640, 480);
    g_window->setWindowTitle("");
    g_window->show();

    // blech
#ifdef _WIN32
    g_window->LoadDirectory(L"C:/");
#else
    g_window->LoadDirectory(L"/");
#endif

    return app.exec();
}

