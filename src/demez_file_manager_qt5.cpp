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
    m_directory->setFixedHeight(24);

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

    // m_viewMode = EFileViewMode::DETAIL;

    // m_fileListView = new FileListDetailView(this);
    // SetViewMode(EFileViewMode::DETAIL);

    // m_fileListIconView = new FileListIconView(this);
    SetViewMode(EFolderViewMode::ICON);

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
}

FileManagerUI::~FileManagerUI()
{
}


void FileManagerUI::BtnNavUp()
{
    fs::path path = QStrToStr(m_directory->text());
    if (path.has_parent_path() && path.parent_path().string() != m_currentDir)
    {
        LoadDirectory(path.parent_path().string());
    }
}

void FileManagerUI::BtnNavGo()
{
    std::string pathStr = QStrToStr(m_directory->text());
    // if (pathStr != m_currentDir)
    {
        LoadDirectory(pathStr);
    }
}


void FileManagerUI::LoadDirectory(const std::string& path)
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
        m_directory->setText(path.c_str());
    }
    catch (fs::filesystem_error)
    {
        printf("Error loading directory \"%s\"", path.c_str());
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

    LoadDirectory(QStrToStr(m_directory->text()));
}


void FileManagerUI::OpenContextMenu(QPoint pos, std::vector<fs::path> items)
{
    // m_cxtMenuHandler->DisplayMenu(pos, m_currentDir.c_str(), items);
    emit CxtLoadMenu(pos, m_currentDir.c_str(), items);
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


void FileManagerUI::RunContextMenuAction()
{
    printf("yoooo\n");

    // OS_RunContextMenuItem(action, m_currentDir.c_str(), m_folderView->GetSelectedFiles());
}


// ================================================================


DirectoryBar::DirectoryBar(QWidget* parent):
    QLineEdit(parent)
{
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
        QLineEdit::keyPressEvent(e);
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

    for (std::string bookmark: m_bookmarkPaths)
    {
        fs::path bookmarkPath = bookmark;
        m_bookmarks->addItem( ToQString(bookmarkPath.filename()) );
    }
}


void Places::LoadDrives()
{
    for (std::string drive: OS_GetMountedDrives())
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

        // FileAddedCallback(file);
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


FileItem::FileItem():
    QStandardItem()
{
}

FileItem::~FileItem()
{
}


// ================================================================


int main(int argc, char* argv[])
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

    g_window->resize(640, 480);
    g_window->setWindowTitle("");
    g_window->show();

    // blech
    g_window->LoadDirectory("C:/");

    return app.exec();
}

