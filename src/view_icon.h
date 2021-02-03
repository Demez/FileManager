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


class FlowLayout;
class FileListIcon;


class FileListIconView: public IBaseFolderView, public QScrollArea
{
public:
    FileListIconView();
    ~FileListIconView();

    // ==================================================

    QWidget* GetWidget() override;

    void DisplayDirectory(const std::wstring& path) override;
    void AddFile(const fs::path& path) override;

    void DeselectAll() override;

    std::vector<fs::path> GetSelectedFiles() override;

    // ==================================================

    void mousePressEvent(QMouseEvent* e) override;

    // QScrollArea* m_scrollArea;
    QWidget* m_scrollAreaContents;
    std::vector<FileListIcon*> m_files;
    FlowLayout* m_layout;
    // QVBoxLayout* m_layout;
    // QVBoxLayout* m_layout;
};


class FileListIcon: public QWidget
{
public:
    FileListIcon(const fs::path& path, QWidget* parent = nullptr);
    ~FileListIcon();

    void LoadIcon();
    void Deselect();
    // void LoadThumbnail();

    void resizeEvent(QResizeEvent* e) override;

    // hover events
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;

    void mousePressEvent(QMouseEvent * e) override;
    void mouseDoubleClickEvent(QMouseEvent * e) override;

    // bool event(QEvent* _event) override;

    bool m_selected = false;
    fs::path m_path;
    QPixmap m_icon;

    QLabel* m_label;
    QLabel* m_iconHolder;
    QVBoxLayout* m_layout;
};


