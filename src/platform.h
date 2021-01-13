#pragma once

#include <string>
#include <filesystem>

#include <QPixmap>
#include <QMenu>
#include <QThread>
#include <QMetaType>

namespace fs = std::filesystem;


// maybe move these enums and structs somewhere else, idk
enum class EIcon
{
    FILE = 0,
    FOLDER,
    COMPUTER,
    NAV_UP,
    NAV_UP_DISABLED,
};


enum class EIconSize
{
    SMALL = 0,
    MEDIUM,
    LARGE,
    EXTRALARGE
};


enum class EFolderViewMode
{
    DETAIL = 0,
    LIST,
    ICON,
};


struct FolderViewSettings
{
    EFolderViewMode viewMode;

    // EViewMode viewMode;
    // EIconSize iconSize;
};


// move somewhere else
enum class EFileType
{
    INVALID = 0,
    FILE,
    FOLDER,
    LINK,
    COUNT
};


// hot take: what if the OS stuff was just a large virtual function class
// and it loaded inheritors of this class from other plugin dlls?
// would easily allow for an internal standalone version, that doesn't try loading anything from anywhere else
// would also be nice to use parts from multiple dlls at a time
// though that might be difficult, but it seems possible

// another hot take:
// if i want to use individual parts from different plugins,
// then i can have like 40 different mini interfaces for those things
// and then in the main interface we have, we have functions to get those interfaces if we want them


int                             OS_Init();

std::vector<std::string>        OS_GetMountedDrives();
std::vector<std::string>        OS_GetBookmarks();

QPixmap                         OS_LoadIcon(EIcon icon);  // this should probably have a size option as well
QPixmap                         OS_LoadIcon(const fs::path& path, EIconSize size);

// windows has a system to make thumbnails, but i have no clue how i would handle it on linux
// QPixmap                         OS_MakeThumbnail(const fs::path& path);

int                             OS_OpenProgram(const std::string& path);

// maybe make this a virtual class? idk
// void                            OS_LoadContextMenu(QMenu* menu, const char* currentDir, std::vector<fs::path> items);
// void                            OS_RunContextMenuItem(fs::path& currentDir);

// void                            OS_ClipboardCopy(const std::string& item);
// void                            OS_ClipboardCut(const std::string& item);
// void                            OS_ClipboardPaste(const fs::path& dir);

// possible folder view settings for windows:
// IShellFolder2::GetDefaultColumn
//


// might use Win32 context menu on windows, seems like it would be a little easier atm
// and have another standalone implementation, uses it's own values
// ...unless you want linux "shell extensions" for certain file browsers
// but then you won't be able to use it on windows, meh
class IContextMenuHandler: public QObject
{
    Q_OBJECT

public slots:

    // NOTE: LoadMenu is called on another thread, but im going to change that so it's not
    // im probably going to get rid of it, it doesn't work with the win32 context menu
    // so just stub LoadMenu and only use DisplayMenu and CloseMenu
    virtual void LoadMenu(QPoint pos, const char* currentDir, std::vector<fs::path> items) = 0;
    virtual void DisplayMenu(QPoint pos, const char* currentDir, std::vector<fs::path> items) = 0;
    virtual void CloseMenu() = 0;

signals:

    void LoadingFinished(QPoint pos, std::vector<fs::path> items);
};


class IFolderViewManager
{
public:
    virtual void GetViewSettings(const char* dir, FolderViewSettings& settings) = 0;
    virtual void SetViewSettings(const char* dir, FolderViewSettings& settings) = 0;
};


// only called once, so no need to use static, just use return new
IContextMenuHandler*            GetIContextMenuHandler();
IFolderViewManager*             GetIFolderViewManager();

