#pragma once

#include <QWidget>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


class IBaseFolderView
{
public:
    virtual ~IBaseFolderView() = 0;

    virtual QWidget* GetWidget() = 0;

    virtual void DisplayDirectory(const std::wstring& path) = 0;
    virtual void AddFile(const fs::path& path) = 0;

    virtual void DeselectAll() = 0;

    virtual std::vector<fs::path> GetSelectedFiles() = 0;
};



