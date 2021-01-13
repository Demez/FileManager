// awful cross platform include file
#include <list>
#include <string>
#include <corecrt_wstring.h>
#include "platform.h"
#include "win_shared.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <Ole2.h>
#include <libloaderapi.h>

#include <QtWinExtras>

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPixmap>
#include <QScopedArrayPointer>
#include <QStringList>
#include <QSysInfo>

#include <iostream>


#ifdef UNICODE
typedef LPCWSTR LPCSTRX;
#else
typedef LPCSTR LPCSTRX;
#endif


/* This example demonstrates the Windows-specific image conversion
* functions. */

struct PixmapEntry {
    QString name;
    QPixmap pixmap;
};

using PixmapEntryList = QVector<PixmapEntry>;


static std::wostream &operator<<(std::wostream &str, const QString &s)
{
    #ifdef Q_OS_WIN
    str << reinterpret_cast<const wchar_t *>(s.utf16());
    #else
    str << s.toStdWString();
    #endif
    return str;
}

static QString formatSize(const QSize &size)
{
    return QString::number(size.width()) + u'x' + QString::number(size.height());
}

static std::string formatSizeStdStr(const QSize &size)
{
    return formatSize(size).toStdString();
}

const char* formatSizeChar(const QSize &size)
{
    std::string str = formatSize(size).toStdString();
    std::string_view strView = str;
    return strView.data();

    // return str.c_str();
}


HINSTANCE g_shell32 = NULL;
int g_iconCount = -1;


int WIN_LoadShell32()
{
    const char* modName = "shell32.dll";

    g_shell32 = LoadLibrary(modName);
    if (!g_shell32)
    {
        WIN_PrintLastError("Failed to load module \"%s\"\n", modName);
        return 1;
    }

    /*
    char shell32Path[512] = {0};
    GetSystemDirectoryA(shell32Path, 512);
    strncat(shell32Path, "\\shell32.dll", 512);
    // -1 returns the number of icons stored in the specified file 
    g_iconCount = (int)ExtractIcon(g_shell32, shell32Path, -1);
    */

    return 0;
}


enum class EWinIcons
{
    FILE = 0,
    FOLDER = 3,
    COMPUTER = 15,
    SYS = 72,

    NAV_UP = 317,
    NAV_UP_DISABLED = 318,

    SETTINGS = 316,
    QUICK_ACCESS = 320,
};


EWinIcons ToWinIcon(EIcon icon)
{
    switch (icon)
    {
    case EIcon::FOLDER:
        return EWinIcons::FOLDER;
    case EIcon::COMPUTER:
        return EWinIcons::COMPUTER;

    case EIcon::NAV_UP:
        return EWinIcons::NAV_UP;
    case EIcon::NAV_UP_DISABLED:
        return EWinIcons::NAV_UP_DISABLED;

    case EIcon::FILE:
    default:
        return EWinIcons::FILE;
    }
}

QPixmap OS_ExtractIcon(const char* path, int index)
{
    // TODO: once you have global settings for the view type and icon size
    // you might want to use ExtractIconExA so you can specify what size you want

    HICON hIcon;

    hIcon = ExtractIconA(g_shell32, path, index);

    QPixmap pixmap;

    if (hIcon)
    {
        pixmap = QtWin::fromHICON(hIcon);
        DestroyIcon(hIcon);
    }

    return pixmap;
}


QPixmap OS_LoadIconShell(int index)
{
    return OS_ExtractIcon("C:\\Windows\\System32\\shell32.dll", index);
}

#ifdef UNICODE
LPCWSTR stringToLPCSTR(const std::string& str)
#else
LPCSTR stringToLPCSTR(const std::string& str)
#endif
{
    #ifdef UNICODE
    std::wstring tmp(str.begin(), str.end());
    LPCWSTR result = tmp.c_str();
    #else
    LPCSTR result = str.c_str();
    #endif
    return result;
}


HICON GetFileIcon(const std::string path, EIconSize type)
{
    HICON hIcon;
    SHFILEINFO sfi={0};
    UINT flag = SHGFI_ICON|SHGFI_USEFILEATTRIBUTES;

    switch (type)
    {
    case EIconSize::SMALL:          flag |= SHGFI_SMALLICON; break;
    case EIconSize::MEDIUM:         flag |= SHGFI_LARGEICON; break;
    case EIconSize::LARGE:
    case EIconSize::EXTRALARGE:     flag |= SHGFI_SYSICONINDEX; break;
    }

    LPCSTR lpPath = stringToLPCSTR(path);
    DWORD_PTR hr = SHGetFileInfoA(lpPath, GetFileAttributesA(lpPath), &sfi, sizeof(sfi), flag);

    if (SUCCEEDED(hr))
    {
        // TODO: large and extra large is broken right now, fun
        if(type == EIconSize::LARGE || type == EIconSize::EXTRALARGE)
        {
            // Retrieve the system image list.
            HIMAGELIST* imageList; 
            hr = SHGetImageList(((type == EIconSize::EXTRALARGE) ? SHIL_JUMBO : SHIL_EXTRALARGE), IID_IImageList, (void**)&imageList);

            if (SUCCEEDED(hr))
            {
                // Get the icon we need from the list. Note that the HIMAGELIST we retrieved
                // earlier needs to be casted to the IImageList interface before use.
                hr = ((IImageList*)imageList)->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon);
            }
        }
        else
        {
            hIcon = sfi.hIcon;
        }
    }
    return hIcon;
}


QPixmap OS_LoadIcon(const fs::path& path, EIconSize size)
{
    HICON hIcon = GetFileIcon(path.string(), size);

    QPixmap pixmap;

    if (hIcon)
    {
        pixmap = QtWin::fromHICON(hIcon);
        DestroyIcon(hIcon);
    }

    return pixmap;
}


QPixmap OS_LoadIcon(EIcon icon)
{
    return OS_LoadIconShell((int)ToWinIcon(icon));
}


