#include <string>

#define WIN32_LEAN_AND_MEAN

#include "platform.h"
#include "win_shared.h"

#include <stdio.h>
#include <Windows.h>
#include <Ole2.h>
#include <ShlObj_core.h>
#include <UserEnv.h>

#include <atlcomcli.h>
#include <string.h>


#define WIN_AD_PATH L"\\microsoft\\windows\\Recent\\AutomaticDestinations\\"
#define WIN_AD_EXT L".automaticDestinations-ms"

#define WIN_AD_QUICK_ACCESS L"f01b4d95cf55d32a"
#define WIN_AD_PINNED_AND_RECENT L"1b4dd67f29cb1962"


std::wstring GetAppDataPath()
{
    wchar_t config[MAX_PATH] = {};
    SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, &config[0]);
    // SHGetKnownFolderPath(nullptr, FOLDERID_ProgramData, nullptr, SHGFP_TYPE_CURRENT, &config[0]);
    return config;
}


std::wstring GetRecycleBinPath()
{
    wchar_t config[MAX_PATH] = {};
    SHGetFolderPath(nullptr, CSIDL_BITBUCKET, nullptr, SHGFP_TYPE_CURRENT, &config[0]);
    return config;
}


std::vector<fs::path> OS_GetBookmarks()
{
    std::vector<fs::path> bookmarks;

    std::wstring quickAccessFile = GetAppDataPath() + WIN_AD_PATH + WIN_AD_QUICK_ACCESS + WIN_AD_EXT;

    HRESULT hr = S_OK;
    IStorage *pStg = NULL;
    // WCHAR *pwszError = L"";

    try
    {
        hr = StgOpenStorageEx( quickAccessFile.c_str(),
                              STGM_READ|STGM_TRANSACTED,
                              STGFMT_STORAGE,
                              0, NULL, NULL, 
                              IID_IStorage,
                              reinterpret_cast<void**>(&pStg) );
        if( FAILED(hr) ) 
            throw L"Failed StgOpenStorageEx";

        IEnumSTATSTG* children = NULL;
        hr = pStg->EnumElements(NULL, NULL, NULL, &children);

        if( FAILED(hr) ) 
            throw L"Failed IPropertySetStorage::Open";

        bool found = false;

        STATSTG child;
        memset(&child, 0, sizeof(child));
        while (hr == S_OK && !found)
        {
            hr = children->Next(1, &child, 0);

            if ( hr != S_OK )
            {
                break;
            }

            if (STGTY_STREAM == child.type)
            {
                IStream* childStream;
                hr = pStg->OpenStream(child.pwcsName, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, &childStream);
                if (SUCCEEDED(hr))
                {
                    IShellLink* childLink;
                    CComPtr<IShellLink> spsl;
                    hr = spsl.CoCreateInstance(CLSID_ShellLink);
                    if (SUCCEEDED(hr))
                    {
                        CComQIPtr<IPersistStream> childPersistStream(spsl);
                        if (childPersistStream && SUCCEEDED(childPersistStream->Load(childStream)))
                        {
                            childLink = spsl.Detach();
                            LPITEMIDLIST absPidl;
                            childLink->GetIDList(&absPidl);
                            if (SUCCEEDED(hr))
                            {
                                IShellItem* targetItem;
                                LPCITEMIDLIST absCPidl = absPidl;
                                hr = SHCreateItemFromIDList(absCPidl, IID_PPV_ARGS(&targetItem));
                                if (SUCCEEDED(hr))
                                {
                                    LPWSTR szTarget;
                                    hr = targetItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szTarget);
                                    if (SUCCEEDED(hr))
                                    {
                                        // std::wstring ws = szTarget;
                                        // bookmarks.push_back( std::string(ws.begin(), ws.end()) );

                                        // fs::path ws = szTarget;
                                        // bookmarks.push_back(ws);

                                        bookmarks.push_back(szTarget);

                                        // removing bookmark, will probably need to use later
                                        /*CString wPathToFolder = CharLower(szTarget);
                                        if (wPathToFolder.Compare(folderParsingNameToLower) == 0)
                                        {
                                            hr = aDestStorageCopy->DestroyElement(child.pwcsName);
                                            if (SUCCEEDED(hr))
                                            {
                                                found = true;
                                            }
                                        }*/
                                    }
                                    targetItem->Release();
                                }
                                CoTaskMemFree(absPidl);
                            }
                            childLink->Release();
                        }
                    }
                    childStream->Release();

                }
            }
        }
    }
    catch( const WCHAR *pwszError )
    {
        wprintf( L"Error:  %s (hr=%08x)\n", pwszError, hr );
    }

    if( pStg ) pStg->Release();

    return bookmarks;
}


/*extern void __cdecl UnpinFromHomeEx(LPWSTR automaticDestFilePath, LPWSTR folderParsingName, LPWSTR tempPath)
{
    LPTSTR folderParsingNameToLower = CharLower(folderParsingName);
    bool found = false;
    IStorage* automaticDestStorage;
    HRESULT hr = StgOpenStorageEx(automaticDestFilePath, STGM_READ | STGM_TRANSACTED, STGFMT_STORAGE, NULL, NULL, NULL, IID_IStorage, (void**)&automaticDestStorage);
    if (SUCCEEDED(hr))
    {
        IStorage* aDestStorageCopy;
        hr = StgCreateStorageEx(tempPath, STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, NULL, NULL, IID_IStorage, (void**)&aDestStorageCopy);
        if (SUCCEEDED(hr))
        {
            hr = automaticDestStorage->CopyTo(NULL, NULL, NULL, aDestStorageCopy);
            if (SUCCEEDED(hr))
            {
                setlocale(LC_ALL, "");
                IEnumSTATSTG* children = NULL;
                HRESULT hr = aDestStorageCopy->EnumElements(NULL, NULL, NULL, &children);
                if (SUCCEEDED(hr))
                {
                    STATSTG child;
                    memset(&child, 0, sizeof(child));
                    hr = children->Next(1, &child, 0);
                    while (hr == S_OK && !found)
                    {
                        if (STGTY_STREAM == child.type)
                        {
                            IStream* childStream;
                            hr = aDestStorageCopy->OpenStream(child.pwcsName, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, &childStream);
                            if (SUCCEEDED(hr))
                            {
                                IShellLink* childLink;
                                CComPtr<IShellLink> spsl;
                                hr = spsl.CoCreateInstance(CLSID_ShellLink);
                                if (SUCCEEDED(hr))
                                {
                                    CComQIPtr<IPersistStream> childPersistStream(spsl);
                                    if (childPersistStream && SUCCEEDED(childPersistStream->Load(childStream)))
                                    {
                                        childLink = spsl.Detach();
                                        LPITEMIDLIST absPidl;
                                        childLink->GetIDList(&absPidl);
                                        if (SUCCEEDED(hr))
                                        {
                                            IShellItem* targetItem;
                                            LPCITEMIDLIST absCPidl = absPidl;
                                            hr = SHCreateItemFromIDList(absCPidl, IID_PPV_ARGS(&targetItem));
                                            if (SUCCEEDED(hr))
                                            {
                                                LPWSTR szTarget;
                                                hr = targetItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &szTarget);
                                                if (SUCCEEDED(hr))
                                                {
                                                    CString wPathToFolder = CharLower(szTarget);
                                                    if (wPathToFolder.Compare(folderParsingNameToLower) == 0)
                                                    {
                                                        hr = aDestStorageCopy->DestroyElement(child.pwcsName);
                                                        if (SUCCEEDED(hr))
                                                        {
                                                            found = true;
                                                        }
                                                    }
                                                }
                                                targetItem->Release();
                                            }
                                            CoTaskMemFree(absPidl);
                                        }
                                        childLink->Release();
                                    }
                                }
                                childStream->Release();
                            }
                        }
                        CoTaskMemFree(child.pwcsName);
                        hr = children->Next(1, &child, 0);
                    }
                    children->Release();
                }
            }
            aDestStorageCopy->Release();
        }
        automaticDestStorage->Release();
        if (found)
        {
            CopyFile(tempPath, automaticDestFilePath, false);
        }
    }
}*/


void OS_LoadBookmarks()
{
}


void OS_AddBookmark(const std::wstring& path)
{
    // ShellExecute(0, L"pintohome", path.c_str(), L"", L"", SW_HIDE);
}


void OS_RemoveBookmark(const std::wstring& path)
{
}


