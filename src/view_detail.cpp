#include "demez_file_manager_qt5.h"
#include "view_detail.h"
#include "platform.h"
#include "util_qt.h"


FileListDetailModel::FileListDetailModel():
    QStandardItemModel()
{
}

FileListDetailModel::~FileListDetailModel()
{
    /*while (rowCount() > 0)
    {
        RemoveItem(rowCount());
    }*/
}


// TODO: all file/directory loading needs to be on another thread
// and not in the model either, so when we switch views, we don't pointlessly reload this
// after the async load, add all the items from whatever class we store that in
// that way, it's still loaded when we switch views
void FileListDetailModel::DisplayDirectory(const std::string& path)
{
}


void FileListDetailModel::AddItem(const fs::path& file)
{
    std::string fileString = file.filename().string();
    QString fileName(fileString.c_str());

    // itemIcon->image

    QStandardItem* itemIcon = new QStandardItem;

    QPixmap pixmap = OS_LoadIcon(file, EIconSize::SMALL);
    if (pixmap.isNull())
    {
        // wtf
        std::string pathStr = file.string();
        printf("failed to load icon for \"%s\"\n", pathStr.c_str());
        // return;
    }
    else
    {
        QIcon icon(pixmap);
        itemIcon->setIcon(icon);
    }

    QStandardItem* itemFileName = new QStandardItem;
    itemFileName->setText(fileName);

    // temp position, will iterate through the directory first,
    // then sort the files, then add the files
    int position = rowCount();

    insertRow(position);
    setItem(position, 0, itemIcon);
    setItem(position, 1, itemFileName);
}


void FileListDetailModel::RemoveItem(const fs::path& file)
{
    for (int i = 0; i < rowCount(); i++)
    {
        // TODO: change this when you make columns movable
        QStandardItem* item = this->item(i, 0);

        if (item->text() == PathToCStr(file))
        {
            RemoveItem(i);
            break;
        }
    }
}


void FileListDetailModel::RemoveItem(int row)
{
    /*for (int i = 0; i < columnCount(); i++)
    {
        // TODO: change this when you make columns movable
        QStandardItem* item = this->item(row, i);

        // if (item)
        {
            delete item;
        }
    }*/

    removeRow(row);
}

// ================================================================


FileListDetailView::FileListDetailView()
{
    m_model = new FileListDetailModel;
    m_view = new QTreeView;
    m_view->setModel(m_model);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->header()->setStretchLastSection(false);
}

FileListDetailView::~FileListDetailView()
{
    delete m_model;
    delete m_view;
}


QWidget* FileListDetailView::GetWidget()
{
    return m_view;
}


FileListDetailModel* FileListDetailView::FileList()
{
    return (FileListDetailModel*)m_view->model();
}


void FileListDetailView::DisplayDirectory(const std::string& path)
{
    // or removeRows(rowCount())
    FileList()->clear();

    // TODO: mix into one call?
    for (fs::path file: g_dirLoader->m_folders)
    {
        FileList()->AddItem(file);
    }

    for (fs::path file: g_dirLoader->m_files)
    {
        FileList()->AddItem(file);
    }

    // TODO: put on another thread aaaa
    /*bool hack = true;
    for (fs::path file: fs::directory_iterator(path))
    {
        if (hack)
        {
            FileList()->clear();
            hack = false;
        }

        FileList()->AddItem(file);
    }*/

    m_view->resizeColumnToContents(1);
    m_view->setColumnWidth(0, 40);  // ugly magic number
    // FileList()->LoadDirectory(path);
}

void FileListDetailView::AddFile(const fs::path& path)
{

}

void FileListDetailView::DeselectAll()
{

}

std::vector<fs::path> FileListDetailView::GetSelectedFiles()
{
    std::vector<fs::path> items;
    return items;
}

