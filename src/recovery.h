#pragma once
#include "storage.h"
#include <QLockFile>
#include <memory>
class Recovery {
public:
    explicit Recovery(QString directory = {});
    bool write(const Note::File &file, QString *error);
    void clear();
    QStringList available() const;
    Note::Result read(const QString &path) const;
    bool remove(const QString &path);
    QString path() const {return path_;}
private:
    QString directory_,path_;
    std::unique_ptr<QLockFile> lock_;
};
