#pragma once

#include <QString>
#include <QLayout>

#include "util.h"

QString ToQString(const std::string& string);
QString ToQString(const fs::path& path);

void QStrToChar(const QString& qString, char* cStr);
std::string QStrToStr(const QString& qString);

void ClearLayout(QLayout *layout);

