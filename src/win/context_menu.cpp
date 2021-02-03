#include "platform.h"
#include "demez_file_manager_qt5.h"

#include <QAction>
#include <QMenu>
#include <QtWinExtras>

#include "win_shared.h"
#include "util.h"

#include <ShObjIdl_core.h>
#include <ShlObj_core.h>


// https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-icontextmenu-querycontextmenu

IContextMenu *g_context = nullptr;

IContextMenu3* g_tmpContext3 = nullptr;
IContextMenu2* g_tmpContext2 = nullptr;
IContextMenu* g_tmpContext = nullptr;

const int g_minShellId = 1;
const int g_maxShellId = 1000;


bool WIN_GetPIDL(const char* cStr, PIDLIST_ABSOLUTE &pidlItem)
{
    std::wstring wStr = StrToWStr(cStr);
    str_replace(wStr, L"/", L"\\"); // needs to be backslashes here
    PCWSTR pcwStr = wStr.c_str();

    // wiki says this should be called from a background thread
    HRESULT hr = SHParseDisplayName(pcwStr, nullptr, &pidlItem, SFGAO_FILESYSTEM, nullptr);

    if (!SUCCEEDED(hr))
    {
        WIN_PrintLastError("Failed to make PIDL for path \"%s\"", cStr);
    }

    return SUCCEEDED(hr);
}


bool WIN_GetPIDL(const wchar_t* cStr, PIDLIST_ABSOLUTE &pidlItem)
{
    std::wstring wStr = cStr;
    str_replace(wStr, L"/", L"\\"); // needs to be backslashes here
    PCWSTR pcwStr = wStr.c_str();

    // wiki says this should be called from a background thread
    HRESULT hr = SHParseDisplayName(pcwStr, nullptr, &pidlItem, SFGAO_FILESYSTEM, nullptr);

    if (!SUCCEEDED(hr))
    {
        WIN_PrintLastError("Failed to make PIDL for path \"%s\"", cStr);
    }

    return SUCCEEDED(hr);
}


// ok this is just a direct copy and paste from explorer++ lmao
HRESULT BindToIdl(PCIDLIST_ABSOLUTE pidl, REFIID riid, void **ppv)
{
    IShellFolder *pDesktop = nullptr;
    HRESULT hr = SHGetDesktopFolder(&pDesktop);

    if (SUCCEEDED(hr))
    {
        /* See http://blogs.msdn.com/b/oldnewthing/archive/2011/08/30/10202076.aspx. */
        // basically some recycle bin thing with multiple files having the same path, idk
        if (pidl->mkid.cb)
        {
            hr = pDesktop->BindToObject(pidl, nullptr, riid, ppv);
        }
        else
        {
            hr = pDesktop->QueryInterface(riid, ppv);
        }

        pDesktop->Release();
    }

    return hr;
}


// workaround to put a nullptr in between the IID_PPV_ARGS macro
HRESULT WIN_GetUIObjectOf(IShellFolder* shellDir, HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, void **ppv)
{
    return shellDir->GetUIObjectOf(hwnd, cidl, apidl, riid, nullptr, ppv);
}


void WIN_ContextMenuAction()
{
    printf("oo diddy\n");
}


// obsolete
void OS_LoadContextMenu(QMenu* menu, const char* currentDir, std::vector<fs::path> items)
{
}


#undef LoadMenu


LRESULT CALLBACK ShellMenuHookProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);



template <typename T,size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define SIZEOF_ARRAY(array)	(sizeof(ArraySizeHelper(array)))


class Win32ContextMenuHandler: public IContextMenuHandler
{
public:

    HMENU m_menu;

    IContextMenu *m_mainCxtMenu = nullptr;

    IContextMenu3* m_cxtMenu3 = nullptr;
    IContextMenu2* m_cxtMenu2 = nullptr;
    IContextMenu* m_cxtMenu = nullptr;

    const int MIN_SHELL_ID = 1;
    const int MAX_SHELL_ID = 1000;
    const int CONTEXT_MENU_SUBCLASS_ID = 1;

    Win32ContextMenuHandler()  {}
    ~Win32ContextMenuHandler() {}

    void LoadMenu(QPoint pos, const wchar_t* currentDir, std::vector<fs::path> items)
    {
        HRESULT hr;
        IContextMenu* tmpContext;
        HWND hwnd = (HWND)g_window->winId();

        PIDLIST_ABSOLUTE pidCurrentDir;
        WIN_GetPIDL(currentDir, pidCurrentDir);

        // No items selected
        if (items.empty())
        {
            IShellFolder* shellCurrentDir;
            PCUITEMID_CHILD pidlRelative = nullptr;
            hr = SHBindToParent(pidCurrentDir, IID_PPV_ARGS(&shellCurrentDir), &pidlRelative);

            if (SUCCEEDED(hr))
            {
                // load the context menu for no selected items
                hr = WIN_GetUIObjectOf(shellCurrentDir, hwnd, 1, &pidlRelative, IID_PPV_ARGS(&tmpContext));
            }
        }
        else
        {
            IShellFolder* shellFolder;
            hr = BindToIdl(pidCurrentDir, IID_PPV_ARGS(&shellFolder));

            if (SUCCEEDED(hr))
            {
                // Load pContextMenu with the items needed
                std::vector<PCITEMID_CHILD> pidlItems;
                for (fs::path item: items)
                {
                    PIDLIST_ABSOLUTE pidlItem;
                    if (WIN_GetPIDL(PathToChar(item), pidlItem))
                    {
                        pidlItems.push_back(pidlItem);
                    }
                }

                hr = WIN_GetUIObjectOf(shellFolder, hwnd, static_cast<UINT>(items.size()),
                    pidlItems.data(), IID_PPV_ARGS(&m_mainCxtMenu));
            }
        }

        if (SUCCEEDED(hr))
        {
            // First, try to get IContextMenu3, then IContextMenu2, and if neither of these are available, IContextMenu.
            hr = tmpContext->QueryInterface(IID_PPV_ARGS(&m_cxtMenu3));
            m_mainCxtMenu = m_cxtMenu3;

            if (FAILED(hr))
            {
                hr = tmpContext->QueryInterface(IID_PPV_ARGS(&m_cxtMenu2));
                m_mainCxtMenu = m_cxtMenu2;

                if (FAILED(hr))
                {
                    hr = tmpContext->QueryInterface(IID_PPV_ARGS(&m_cxtMenu));
                    m_mainCxtMenu = m_cxtMenu;
                }
            }
        }

        m_menu = CreatePopupMenu();

        if (m_menu == nullptr)
        {
            return;
            // return E_FAIL;
        }

        UINT flags = CMF_NORMAL;

        // ok this NEEDS to be on another thread, this can take a second or two lmao
        hr = m_mainCxtMenu->QueryContextMenu(m_menu, 0, g_minShellId, g_maxShellId, flags);

        if (!SUCCEEDED(hr))
        {
            // might just be out of items in the menu, idk
            WIN_PrintLastError("Failed to query context menu");
            return;
        }

        // emit LoadingFinished(pos, items);
    }

    void DisplayMenu(QPoint pos, const wchar_t* currentDir, std::vector<fs::path> items)
    {
        // watch it break because it's on the main thread now
        printf("display context menu moment");

        HWND hwnd = (HWND)g_window->winId();

        BOOL bWindowSubclassed = FALSE;

        if (m_cxtMenu3 != nullptr || m_cxtMenu2 != nullptr)
        {
            // Subclass the owner window, so that the shell can handle menu messages.
            bWindowSubclassed = SetWindowSubclass(hwnd, ShellMenuHookProcStub,
                CONTEXT_MENU_SUBCLASS_ID, reinterpret_cast<DWORD_PTR>(this));
        }

        int iCmd = TrackPopupMenu(m_menu, TPM_LEFTALIGN | TPM_RETURNCMD, pos.x(), pos.y(), 0, hwnd, nullptr);

        if (bWindowSubclassed)
        {
            // Restore previous window procedure.
            RemoveWindowSubclass(hwnd, ShellMenuHookProcStub, CONTEXT_MENU_SUBCLASS_ID);
        }

        HRESULT hr;
        if (iCmd >= MIN_SHELL_ID && iCmd <= MAX_SHELL_ID)
        {
            TCHAR szCmd[64];
            hr = m_mainCxtMenu->GetCommandString(iCmd - MIN_SHELL_ID, GCS_VERB, nullptr, reinterpret_cast<LPSTR>(szCmd), SIZEOF_ARRAY(szCmd));

            BOOL bHandled = FALSE;

            // Pass the menu back to the caller to give
            // it the chance to handle it.
            if (SUCCEEDED(hr))
            {
                // bHandled = HandleMenuItem(m_pidlParent.get(), m_pidlItems, dwData, szCmd);
            }

            if (!bHandled)
            {
                CMINVOKECOMMANDINFO cmici;

                cmici.cbSize = sizeof(CMINVOKECOMMANDINFO);
                cmici.fMask = 0;
                cmici.hwnd = hwnd;
                cmici.lpVerb = (LPCSTR) MAKEWORD(iCmd - MIN_SHELL_ID, 0);
                cmici.lpParameters = nullptr;
                cmici.lpDirectory = nullptr;
                cmici.nShow = SW_SHOW;

                m_mainCxtMenu->InvokeCommand(&cmici);
            }

        }

        // emit a delete menu signal?
        CloseMenu();
    }

    void CloseMenu()
    {
        if (m_menu)             DestroyMenu(m_menu);
        // if (m_mainCxtMenu)      delete m_mainCxtMenu;
        // if (m_cxtMenu3)         delete m_cxtMenu3;
        // if (m_cxtMenu2)         delete m_cxtMenu2;
        // if (m_cxtMenu)          delete m_cxtMenu;
    }

    // yes i know this is a copy and paste idc
    LRESULT CALLBACK ShellMenuHookProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            case WM_MEASUREITEM:
                /* wParam is 0 if this item was sent by a menu. */
                if (wParam == 0)
                {
                    if (m_cxtMenu3 != nullptr)
                    {
                        m_cxtMenu3->HandleMenuMsg2(uMsg, wParam, lParam, nullptr);
                    }
                    else if (m_cxtMenu2 != nullptr)
                    {
                        m_cxtMenu2->HandleMenuMsg(uMsg, wParam, lParam);
                    }

                    return TRUE;
                }
                break;

            case WM_DRAWITEM:
                if (wParam == 0)
                {
                    if (m_cxtMenu3 != nullptr)
                    {
                        m_cxtMenu3->HandleMenuMsg2(uMsg, wParam, lParam, nullptr);
                    }
                    else if (m_cxtMenu2 != nullptr)
                    {
                        m_cxtMenu2->HandleMenuMsg(uMsg, wParam, lParam);
                    }
                }
                return TRUE;
                break;

            case WM_INITMENUPOPUP:
            {
                if (m_cxtMenu3 != nullptr)
                {
                    m_cxtMenu3->HandleMenuMsg2(uMsg, wParam, lParam, nullptr);
                }
                else if (m_cxtMenu2 != nullptr)
                {
                    m_cxtMenu2->HandleMenuMsg(uMsg, wParam, lParam);
                }
            }
            break;

            case WM_MENUSELECT:
            {
                /*if (m_pStatusBar != nullptr)
                {
                    if (HIWORD(wParam) == 0xFFFF && lParam == 0)
                    {
                        m_pStatusBar->HandleStatusBarMenuClose();
                    }
                    else
                    {
                        m_pStatusBar->HandleStatusBarMenuOpen();

                        int iCmd = static_cast<int>(LOWORD(wParam));

                        if (!((HIWORD(wParam) & MF_POPUP) == MF_POPUP)
                            && (iCmd >= m_iMinID && iCmd <= m_iMaxID))
                        {
                            TCHAR szHelpString[512];

                            // Ask for the help string for the currently selected menu item.
                            HRESULT hr = m_mainCxtMenu->GetCommandString(iCmd - m_iMinID, GCS_HELPTEXT,
                                nullptr, reinterpret_cast<LPSTR>(szHelpString), SIZEOF_ARRAY(szHelpString));

                            // If the help string was found, send it to the status bar.
                            if (hr == NOERROR)
                            {
                                m_pStatusBar->SetPartText(0, szHelpString);
                            }
                        }
                    }

                    // Prevent the message from been passed onto the original window.
                    return 0;
                }*/
            }
            break;
        }

        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }

    bool HandleMenuItem(PCIDLIST_ABSOLUTE pidlParent, const std::vector<PITEMID_CHILD> &pidlItems, int iCmd)
    {

    }

};


LRESULT CALLBACK ShellMenuHookProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    Win32ContextMenuHandler* menuHandler = reinterpret_cast<Win32ContextMenuHandler*>(dwRefData);
    return menuHandler->ShellMenuHookProc(hwnd, msg, wParam, lParam);
}


IContextMenuHandler* GetIContextMenuHandler()
{
    return new Win32ContextMenuHandler;
}


/*void OS_RunContextMenuItem(QAction* action, const char* currentDir, std::vector<fs::path> items)
{
    // idk if i can even call this yet, but whatever
    int iCmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD, ppt->x, ppt->y, 0, m_hwnd, nullptr);

    hr = g_context->GetCommandString(
                iCmd - iMinID, GCS_VERB, nullptr, reinterpret_cast<LPSTR>(szCmd), SIZEOF_ARRAY(szCmd));

    printf("bruh\n");
}*/


/*

hr = m_mainCxtMenu->GetCommandString(
            iCmd - iMinID, GCS_VERB, nullptr, reinterpret_cast<LPSTR>(szCmd), SIZEOF_ARRAY(szCmd));


void ContextMenuManager::InvokeMenuEntry(HWND hwnd, UINT uCmd)
{
    for (auto menuHandler : m_MenuHandlers)
    {
        if (uCmd >= menuHandler.uStartID && uCmd < menuHandler.uEndID)
        {
            CMINVOKECOMMANDINFO cmici;
            cmici.cbSize = sizeof(CMINVOKECOMMANDINFO);
            cmici.fMask = 0;
            cmici.hwnd = hwnd;
            cmici.lpVerb = reinterpret_cast<LPCSTR>(MAKEWORD(uCmd - menuHandler.uStartID, 0));
            cmici.lpParameters = nullptr;
            cmici.lpDirectory = nullptr;
            cmici.nShow = SW_SHOW;

            menuHandler.pContextMenuActual->InvokeCommand(&cmici);
            break;
        }
    }
}
*/


