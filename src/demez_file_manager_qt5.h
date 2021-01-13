#pragma once

#include <QString>
#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include <QListWidget>
#include <QBoxLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QLabel>
#include <QSizePolicy>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>
#include <QPoint>

#include <filesystem>

namespace fs = std::filesystem;

#include "flow_layout.h"
#include "view_detail.h"
#include "view_icon.h"
#include "platform.h"


QString ToQString(const std::string& string);
QString ToQString(const fs::path& path);


// class FileListDetailView;
// class FileListIconView;
class DirectoryBar;
class Places;


class FileManagerUI: public QWidget
{
    Q_OBJECT

public:
    FileManagerUI(QWidget *parent = nullptr);
    ~FileManagerUI();

    void BtnNavUp();
    void BtnNavGo();

    void LoadDirectory(const std::string& path);
    void SetViewMode(EFolderViewMode mode);

    void OpenContextMenu(QPoint pos, std::vector<fs::path> items);
    void RunContextMenuAction();

    Places* m_places;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_navBarLayout;
    QHBoxLayout* m_toolBarLayout;
    QHBoxLayout* m_splitLayout;
    QVBoxLayout* m_fileViewLayout;

    DirectoryBar* m_directory;
    QScrollArea* m_fileViewScroll;

    EFolderViewMode m_viewMode;
    IBaseFolderView* m_folderView;  // might have to move this if i setup tabs, idk

    QMenu* m_cxtMenu = nullptr;
    IContextMenuHandler* m_cxtMenuHandler = nullptr;
    QThread* m_cxtMenuThread = nullptr;

    IFolderViewManager* m_viewManager = nullptr;

    std::vector<fs::path> m_pathHistory;

    std::string m_currentDir;

public slots:
    void DisplayContextMenu(QPoint pos, std::vector<fs::path> items);

signals:
    void CxtLoadMenu(QPoint pos, const char* currentDir, std::vector<fs::path> items);
    // void CxtDisplayMenu(QPoint pos, const char* currentDir, std::vector<fs::path> items);
    void CxtTest(std::vector<fs::path> item);

};


extern FileManagerUI* g_window;


// called BreadCrumbBar in windows, to select folders to go back to
class DirectoryBar: public QLineEdit
{
public:
    DirectoryBar(QWidget* parent = nullptr);
    ~DirectoryBar();

    void keyPressEvent(QKeyEvent* e);
};


class Places: public QWidget
{
public:
    Places(QWidget* parent = nullptr);
    ~Places();

    void LoadPlaces();
    void LoadDrives();
    void LoadBookmarks();

    QListWidget* m_drives;
    QListWidget* m_bookmarks;

    std::vector<std::string> m_bookmarkPaths;
};


// should not be in here but whatever
struct File
{
    std::string m_name;
    EFileType m_type;
};


class DirectoryLoader
{
public:
    DirectoryLoader();
    ~DirectoryLoader();

    void LoadDirectory(const fs::path& path);

    // std::vector<File> m_files;
    std::vector<fs::path> m_files;
    std::vector<fs::path> m_folders;
};


extern DirectoryLoader* g_dirLoader;


class NavButton: public QPushButton
{
public:
    NavButton(const char* name, QWidget* parent = nullptr);
    ~NavButton();
};


// junk
class FileItem: public QStandardItem
{
public:
    FileItem();
    ~FileItem();

    fs::path m_path;
};

