#include "util_qt.h"

#include <QWidget>


// dumb
QString ToQString(const std::string& string)
{
    const char* pathChar = string.c_str();
    QString qString(pathChar);
    return qString;
}

QString ToQString(const fs::path& path)
{
    return ToQString(path.string());
}

void QStrToChar(const QString& qString, char* cStr)
{
    QByteArray ba = qString.toLocal8Bit();
    const char* buf = ba.data();
    strncpy(cStr, buf, ba.size());
}

std::string QStrToStr(const QString& qString)
{
    QByteArray ba = qString.toLocal8Bit();
    const char* buf = ba.data();
    std::string result = buf;
    return result;
}

void ClearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            ClearLayout(item->layout());
            delete item->layout();
        }

        if (item->widget())
        {
            delete item->widget();
        }

        delete item;
    }
}

