#include "demez_file_manager_qt5.h"
#include "view_icon.h"
#include "platform.h"
#include "util_qt.h"


FileListIcon::FileListIcon(const fs::path& path, QWidget* parent):
    QWidget(parent)
{
    m_path = path;

    m_layout = new QVBoxLayout;
    m_layout->setAlignment(Qt::AlignCenter);

    setLayout(m_layout);

    m_iconHolder = new QLabel;
    m_iconHolder->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_iconHolder);

    LoadIcon();

    // setup text (should not be in this function)
    m_label = new QLabel(this);
    m_label->setText(ToQString(path.filename().string()));
    m_label->setAlignment(Qt::AlignCenter);
    // m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_label->setWordWrap(true);

    m_layout->addWidget(m_label);
    // m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(0);

    // this where the global settings will come in
    // setFixedSize(80, 96);
    // setMinimumSize(80, 72);
    setMinimumHeight(40);
    setFixedWidth(80);
    m_label->setMaximumWidth(80);
    m_label->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);

    setAutoFillBackground(true);

    // setStyleSheet("border: 2px solid; border-color: #660000;");
    // setStyleSheet("hover:!pressed { background-color: #660000; }");
}

FileListIcon::~FileListIcon()
{
    // do i need to delete the layout or will qt do it for me?
    delete m_layout;
    delete m_label;
    delete m_iconHolder;
}


void FileListIcon::LoadIcon()
{
    // IDEA: have some global settings singleton,
    // can grab icon sizes from that, etc.

    m_icon = OS_LoadIcon(m_path, EIconSize::MEDIUM);
    if (m_icon.isNull())
    {
        printf("failed to load icon for \"%s\"\n", PathToCStr(m_path));
        return;
    }

    m_iconHolder->setPixmap(m_icon);
}


void FileListIcon::resizeEvent(QResizeEvent* e)
{
    return;
}


// blech
#define COLOR_HOVER "background: #a1dcff;"
#define COLOR_SELECTED "background: #5cc2ff;"
#define COLOR_SELECTED_HOVER "background: #4dbcff;"


// hover events
void FileListIcon::enterEvent(QEvent* e)
{
    // setStyleSheet("border: 2px solid; border-color: #660000;");

    if (m_selected)
    {
        setStyleSheet(COLOR_SELECTED_HOVER);
    }
    else
    {
        setStyleSheet(COLOR_HOVER);
    }
}

void FileListIcon::leaveEvent(QEvent* e)
{
    // setStyleSheet("border: 0px none;");

    if (m_selected)
    {
        setStyleSheet(COLOR_SELECTED);
    }
    else
    {
        setStyleSheet("background: transparent;");
    }
}

void FileListIcon::mousePressEvent(QMouseEvent* e)
{
    // if ctrl + left button, deselect
    if (e->modifiers() & Qt::ControlModifier && e->button() == Qt::LeftButton)
    {
        m_selected = true;
        setStyleSheet(COLOR_SELECTED_HOVER);
    }
    else if (e->button() == Qt::LeftButton)
    {
        g_window->m_folderView->DeselectAll();
        m_selected = true;
        setStyleSheet(COLOR_SELECTED_HOVER);
    }
    else if (e->button() == Qt::RightButton)
    {
        // setContextMenuPolicy(Qt::DefaultContextMenu);

        g_window->OpenContextMenu(e->screenPos().toPoint(), g_window->m_folderView->GetSelectedFiles());

        // open a context menu here?
        // m_selected = true;
        // setStyleSheet(COLOR_SELECTED_HOVER);
    }
}

void FileListIcon::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        try
        {
            if (fs::is_directory(m_path))
            {
                g_window->LoadDirectory(m_path.string());
            }
            else
            {
                OS_OpenProgram(m_path.string());
                // g_window->OpenFile(m_path.string());
            }
        }
        catch (fs::filesystem_error)
        {
            printf("Error checking \"%s\"\n", PathToCStr(m_path));
        }
    }
}


void FileListIcon::Deselect()
{
    m_selected = false;
    setStyleSheet("background: transparent;");
}


// ================================================================


FileListIconView::FileListIconView():
    QScrollArea()
{
    // m_scrollArea = new QScrollArea;

    setContentsMargins(0, 0, 0, 0);
    setWidgetResizable(true);
    // m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    m_scrollAreaContents = new QWidget;
    m_layout = new FlowLayout;
    // m_layout->addStretch(0);

    setWidget(m_scrollAreaContents);
    setLayout(m_layout);

    // m_scrollArea->setLayout(m_layout);
    m_scrollAreaContents->setLayout(m_layout);
    m_scrollAreaContents->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // connect(m_scrollArea, &QWidget::mousePressEvent, this, &FileListIconView::mousePressEvent);

    // m_scrollArea->setWidget(m_scrollAreaContents);

    // m_scrollArea->setStyleSheet("border: 2px solid; border-color: #006600;");
}


FileListIconView::~FileListIconView()
{
    ClearLayout(m_layout);

    delete m_layout;
    delete m_scrollAreaContents;
}


QWidget* FileListIconView::GetWidget()
{
    return this;
}


void FileListIconView::DeselectAll()
{
    // this seems slow lol
    for (FileListIcon* file: m_files)
    {
        file->Deselect();
    }
}


std::vector<fs::path> FileListIconView::GetSelectedFiles()
{
    std::vector<fs::path> files;

    for (FileListIcon* file: m_files)
    {
        if (file->m_selected)
        {
            files.push_back(file->m_path);
        }
    }

    return files;
}


void FileListIconView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
    {
        DeselectAll();
    }

    if (e->button() == Qt::RightButton)
    {
        g_window->OpenContextMenu(e->screenPos().toPoint(), GetSelectedFiles());
    }
}


void FileListIconView::DisplayDirectory(const std::string& path)
{
    ClearLayout(m_layout);
    m_files.clear();

    // TODO: mix into one call?
    for (fs::path file: g_dirLoader->m_folders)
    {
        AddFile(file);
    }

    for (fs::path file: g_dirLoader->m_files)
    {
        AddFile(file);
    }
}


void FileListIconView::AddFile(const fs::path& path)
{
    std::string fileString = path.filename().string();
    QString fileName(fileString.c_str());

    FileListIcon* fileListIcon = new FileListIcon(path, this);

    m_layout->addWidget(fileListIcon);
    m_files.push_back(fileListIcon);
}

