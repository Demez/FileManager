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

#include <filesystem>

namespace fs = std::filesystem;

#include "ibase_folder_view.h"
#include "flow_layout.h"

class FileListDetailView;
class FileListIconView;
class DirectoryBar;
class Places;


struct FileDetail
{
    fs::path m_path;
    QPixmap m_icon;
};


class FileListDetailModel: public QStandardItemModel
{
public:
    FileListDetailModel();
    ~FileListDetailModel();

    void DisplayDirectory(const std::wstring& path);
    void AddItem(const fs::path& file);
    void RemoveItem(const fs::path& file);
    void RemoveItem(int row);

    bool event(QEvent* e) override;

    std::vector<FileDetail*> m_files;
};


// testing
class FileDetailView: public QAbstractItemView
{
public:

};


class FileListDetailView: public IBaseFolderView
{
public:
    FileListDetailView();
    ~FileListDetailView() override;

    // ==================================================

    QWidget* GetWidget() override;

    void DisplayDirectory(const std::wstring& path) override;
    void AddFile(const fs::path& path) override;

    void DeselectAll() override;

    std::vector<fs::path> GetSelectedFiles() override;

    // ==================================================

    void OnEvent(QEvent* e);

    FileListDetailModel* FileList();

    QTreeView*           m_view;
    FileListDetailModel* m_model;
};

