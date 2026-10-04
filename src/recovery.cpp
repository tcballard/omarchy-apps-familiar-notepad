#include "recovery.h"
#include <QStandardPaths>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QUuid>
Recovery::Recovery(QString dir) : directory_(dir.isEmpty()?QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/recovery":dir) {
    QDir().mkpath(directory_);QFile::setPermissions(directory_,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner);
    path_=directory_+"/"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".json";
    lock_=std::make_unique<QLockFile>(path_+".lock");lock_->tryLock(0);
}
bool Recovery::write(const Note::File &f,QString *error) {
    if(!lock_->isLocked()){*error="Recovery directory is not writable.";return false;}
    if(f.text.size()>Note::MaxBytes){*error="Recovery snapshot exceeds the preview limit.";return false;}
    QJsonObject obj{{"version",1},{"path",f.path},{"text",f.text},{"hash",QString::fromLatin1(f.hash)},{"encoding",int(f.encoding)},{"ending",int(f.ending)},{"saved",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}};
    QSaveFile file(path_);file.setDirectWriteFallback(false);
    if(!file.open(QIODevice::WriteOnly)){*error=file.errorString();return false;}
    file.setPermissions(QFile::ReadOwner|QFile::WriteOwner);auto bytes=QJsonDocument(obj).toJson(QJsonDocument::Compact);
    if(file.write(bytes)!=bytes.size() || !file.commit()){*error=file.errorString();return false;}return true;
}
void Recovery::clear(){QFile::remove(path_);}
QStringList Recovery::available() const {
    QStringList out;for(const auto &name:QDir(directory_).entryList({"*.json"},QDir::Files,QDir::Time)){
        const QString path=directory_+"/"+name;if(path==path_)continue;QLockFile lock(path+".lock");if(lock.tryLock(0))out<<path;
    }return out;
}
Note::Result Recovery::read(const QString &path) const {
    Note::Result r;QLockFile lock(path+".lock");if(!lock.tryLock(0)){r.error="This draft belongs to a running Notepad window.";return r;}
    QFile file(path);if(!file.open(QIODevice::ReadOnly) || file.size()>Note::MaxBytes*6+4096){r.error="Recovery draft could not be read.";return r;}
    QJsonParseError err;auto doc=QJsonDocument::fromJson(file.readAll(),&err);auto o=doc.object();
    if(err.error!=QJsonParseError::NoError || o.value("version").toInt()!=1 || !o.value("text").isString() || o.value("text").toString().size()>Note::MaxBytes || o.value("encoding").toInt(-1)<0 || o.value("encoding").toInt()>3 || o.value("ending").toInt(-1)<0 || o.value("ending").toInt()>2){r.error="Invalid recovery draft.";return r;}
    r.file={o.value("path").toString(),o.value("text").toString(),o.value("hash").toString().toLatin1(),Note::Encoding(o.value("encoding").toInt()),Note::Ending(o.value("ending").toInt())};r.ok=true;return r;
}
bool Recovery::remove(const QString &path){QLockFile lock(path+".lock");return lock.tryLock(0)&&QFile::remove(path);}
