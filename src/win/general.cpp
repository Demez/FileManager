#include <vector>
#include <string>
#include "demez_file_manager_qt5.h"
#include "platform.h"
#include "win_shared.h"

#include <stdio.h>
#include <intrin.h>

#include <Windows.h>
#include <fileapi.h>
#include <shellapi.h>
#include <processthreadsapi.h>


std::wstring StrToWStr(const std::string& str)
{
    std::wstring tmp(str.begin(), str.end());
    return tmp;
}


std::string wchar_to_string(wchar_t* wchar)
{
    std::wstring ws(wchar);
    std::string test(ws.begin(), ws.end());
    return test;
}


LPCSTR PathToLPCSTR(const fs::path& str)
{
    static std::string string = str.string();
    return string.c_str();
}


void WIN_PrintLastError(const char* format, ...)
{
    char userErrorMessage[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(userErrorMessage, 1024, format, args);
    va_end(args);

    DWORD dLastError = GetLastError();
    LPCTSTR strErrorMessage = NULL;

    FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                NULL,
                dLastError,
                0,
                (LPSTR) &strErrorMessage,
                0,
                NULL);

    // OutputDebugString(strErrorMessage);
    printf("[ERROR] %s\nWin32 API Error %d: %s\n", userErrorMessage, dLastError, strErrorMessage);
}


#define INIT_FUNC(Func) \
    ret = Func(); \
    if (ret != 0) \
    { \
        return ret; \
    }

int OS_Init()
{
    int ret = 0;

    INIT_FUNC(WIN_LoadShell32)

    return 0;
}

#undef INIT_FUNC


std::vector<std::string> OS_GetMountedDrives()
{
    std::vector<std::string> mountedDrives;

    WCHAR wcharDriveLetters[512];
    GetLogicalDriveStringsW(512-1, wcharDriveLetters);

    for (int i = 0; i < sizeof(wcharDriveLetters); i += 4)
    {
        std::string driveLetters = wchar_to_string(&wcharDriveLetters[i]);
        if (driveLetters.length() == 3)
        {
            mountedDrives.push_back(driveLetters);
        }
        else
        {
            break;
        }
    }

    return mountedDrives;
}


int OS_OpenProgram(const std::string& path)
{
    ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);

    // STARTUPINFO info={sizeof(info)};
    // PROCESS_INFORMATION processInfo;
    // CreateProcessA(NULL, path.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &info, &processInfo);

    return 0;
}


HWND WIN_GetHWND()
{
    return (HWND)g_window->winId();
}


// folder view settings
// https://www.codeproject.com/Articles/20853/Win32-Tips-and-Tricks#browse1
// undocumented message to get IShellBrowser from the file dialog

#define WM_GETISHELLBROWSER (WM_USER+7)

// last 2 don't work on win 2000
static int SH_VIEW_LARGE_ICONS = 28713;
static int SH_VIEW_SMALL_ICONS = 28714;
static int SH_VIEW_LIST = 28715;
static int SH_VIEW_DETAILS = 28716;
static int SH_VIEW_THUMBNAILS = 28717;
static int SH_VIEW_TILES = 28718;


// seems a bit excessive
class Win32FolderViewManager: public IFolderViewManager
{
public:

    void GetViewSettings(const char* dir, FolderViewSettings& settings)
    {
        IShellBrowser *shBrowser = (IShellBrowser*)SendMessage(WIN_GetHWND(), WM_GETISHELLBROWSER, 0, 0);
        IShellView *shView = NULL;

        if (shBrowser->QueryActiveShellView(&shView) == S_OK)
        {
            FOLDERSETTINGS winSettings;

            // doesn't support thumbnails view on windows 2000?
            shView->GetCurrentInfo(&winSettings);

            FOLDERVIEWMODE viewMode = (FOLDERVIEWMODE)winSettings.ViewMode;
            settings.viewMode = GetViewMode(viewMode);

            shView->Release();
        }

        // experiments show that shBrowser doesn't need to be Released

        // get width of name column
        /*HWND hwndHeader = ListView_GetHeader(hwndList);
        if (hwndHeader)
        {
            HDITEM item;
            item.mask=HDI_WIDTH;
            if (Header_GetItem(hwndHeader,0,&item))
            {
                g_NameWidth = item.cxy;
            }
        }*/
    }

    void SetViewSettings(const char* dir, FolderViewSettings& settings)
    {
        // currently restores view settings
        IShellBrowser *shBrowser = (IShellBrowser*)SendMessage(WIN_GetHWND(), WM_GETISHELLBROWSER, 0, 0);
        IShellView *shView = NULL;

        if (shBrowser->QueryActiveShellView(&shView)==S_OK)
        {
            IFolderView *folder = NULL; // requires Win XP

            if (shView->QueryInterface(IID_IFolderView, (void **)&folder) == S_OK)
            {
                folder->SetCurrentViewMode(GetViewMode(settings.viewMode));
                folder->Release();
            }

            shView->Release();
        }

        // restore width of column
        /*HWND hwndHeader = ListView_GetHeader(hwndList);
        if (hwndHeader)
        {
            if ((GetWindowLong(hwndHeader, GWL_STYLE) & WS_VISIBLE) != 0)
            {
                ListView_SetColumnWidth(hwndList, 0, g_NameWidth);
            }
            else
            {
                HDITEM item;
                item.mask = HDI_WIDTH;
                item.cxy = g_NameWidth;
                Header_SetItem(hwndHeader, 0, &item);
            }
        }*/
    }

private:

    EFolderViewMode GetViewMode(FOLDERVIEWMODE viewMode)
    {
        switch (viewMode)
        {
        case FOLDERVIEWMODE::FVM_ICON:
        case FOLDERVIEWMODE::FVM_SMALLICON:
            return EFolderViewMode::ICON;

        case FOLDERVIEWMODE::FVM_DETAILS:
        case FOLDERVIEWMODE::FVM_AUTO:
        default:
            return EFolderViewMode::DETAIL;
        }
    }

    FOLDERVIEWMODE GetViewMode(EFolderViewMode viewMode)
    {
        switch (viewMode)
        {
        case EFolderViewMode::ICON:
            return FOLDERVIEWMODE::FVM_SMALLICON;

        case EFolderViewMode::DETAIL:
        default:
            return FOLDERVIEWMODE::FVM_DETAILS;
        }
    }

};


IFolderViewManager* GetIFolderViewManager()
{
    return new Win32FolderViewManager;
}



