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

    void LoadDirectory(const std::wstring& path);
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
    IBaseFolderView* m_folderView = nullptr;  // might have to move this if i setup tabs, idk

    QMenu* m_cxtMenu = nullptr;
    IContextMenuHandler* m_cxtMenuHandler = nullptr;
    QThread* m_cxtMenuThread = nullptr;

    IFolderViewManager* m_viewManager = nullptr;

    std::vector<fs::path> m_pathHistory;

    std::wstring m_currentDir;

public slots:
    void DisplayContextMenu(QPoint pos, std::vector<fs::path> items);
    void HandleFocusChange(QWidget *old, QWidget *now);

signals:
    void CxtLoadMenu(QPoint pos, const wchar_t* currentDir, std::vector<fs::path> items);
    // void CxtDisplayMenu(QPoint pos, const char* currentDir, std::vector<fs::path> items);
    void CxtTest(std::vector<fs::path> item);

};


extern FileManagerUI* g_window;


class DirectoryBarButton: public QLabel
{
public:
    DirectoryBarButton(fs::path& part);
    ~DirectoryBarButton();

    void SetPath(fs::path part);

    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent * e) override;

    fs::path m_path;
    // QLabel* m_label = nullptr;
};


class DirectoryBar;

class DirectoryBarEdit: public QLineEdit
{
public:
    DirectoryBarEdit(DirectoryBar* dirBar);
    ~DirectoryBarEdit();

    void keyPressEvent(QKeyEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

    fs::path m_path;
    DirectoryBar* m_dirBar = nullptr;
};


// called BreadCrumbBar in windows, to select folders to go back to
class DirectoryBar: public QWidget
{
public:
    DirectoryBar(QWidget* parent = nullptr);
    ~DirectoryBar();

    QString GetPath();
    void SetPath(const fs::path& path);
    void AddPathPartButton(fs::path&);
    void ShowPathEdit();
    void HidePathEdit();

    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent * e) override;

    QHBoxLayout* m_layout = nullptr;
    QHBoxLayout* m_lineLayout = nullptr;

    DirectoryBarEdit* m_lineEdit = nullptr;

    fs::path m_path;
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

    std::vector<fs::path> m_bookmarkPaths;
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


