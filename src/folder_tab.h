#pragma once

#include <QString>
#include <QApplication>
#include <QPushButton>
#include <QMainWindow>


// maybe FolderViewManager or FolderViewContainer would work better? idk
class FolderTab: public QWidget
{

public:
    FolderTab(QWidget *parent);
    ~FolderTab();
};

