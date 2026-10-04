#include "storage.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QLockFile>
#include <QSaveFile>
#include <QStringConverter>
namespace Note {
QString encodingName(Encoding e) { switch(e) {case Encoding::Utf8:return "UTF-8";case Encoding::Utf8Bom:return "UTF-8 BOM";case Encoding::Utf16LE:return "UTF-16 LE";case Encoding::Utf16BE:return "UTF-16 BE";} return {}; }
QString endingName(Ending e) { return e==Ending::CRLF ? "Windows (CRLF)" : e==Ending::CR ? "Classic Mac (CR)" : "Unix (LF)"; }
QByteArray digest(const QByteArray &b) { return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex(); }
Result decode(const QByteArray &bytes) {
    Result r;
    if(bytes.size()>MaxBytes) {r.error="This preview opens text files up to 8 MiB.";return r;}
    QByteArray data=bytes; auto codec=QStringConverter::Utf8;
    if(data.startsWith(QByteArray::fromHex("efbbbf"))) {r.file.encoding=Encoding::Utf8Bom;data.remove(0,3);}
    else if(data.startsWith(QByteArray::fromHex("fffe"))) {r.file.encoding=Encoding::Utf16LE;codec=QStringConverter::Utf16LE;data.remove(0,2);}
    else if(data.startsWith(QByteArray::fromHex("feff"))) {r.file.encoding=Encoding::Utf16BE;codec=QStringConverter::Utf16BE;data.remove(0,2);}
    QStringDecoder decoder(codec, QStringConverter::Flag::Stateless | QStringConverter::Flag::ConvertInitialBom);QString text=decoder.decode(data);
    if(decoder.hasError() || ((codec==QStringConverter::Utf16LE || codec==QStringConverter::Utf16BE) && data.size()%2)) {r.error="This file is not valid UTF-8 or BOM-marked UTF-16. Its bytes have not been changed.";return r;}
    if(text.contains(QChar(0)) || text.contains(QChar(0x2028)) || text.contains(QChar(0x2029))) {r.error="This file contains binary or unsupported paragraph separators. Its bytes have not been changed.";return r;}
    const int crlf=text.count("\r\n");QString rest=text;rest.remove("\r\n");const bool lf=rest.contains('\n'),cr=rest.contains('\r');
    if((crlf>0)+int(lf)+int(cr)>1) {r.error="Mixed line endings are not supported in this preview. Normalize a copy before opening; the original is unchanged.";return r;}
    r.file.ending=crlf ? Ending::CRLF : cr ? Ending::CR : Ending::LF;
    text.replace("\r\n","\n");text.replace('\r','\n');
    r.file.text=text;r.file.hash=digest(bytes);r.ok=true;return r;
}
QByteArray encode(const QString &text, Encoding e, Ending ending, QString *error) {
    if(text.size()>MaxBytes || text.contains(QChar(0)) || text.contains(QChar(0x2028)) || text.contains(QChar(0x2029))) {*error="Text is too large or contains unsupported control characters.";return {};}
    QString normalized=text; normalized.replace("\r\n","\n");normalized.replace('\r','\n');
    if(ending==Ending::CRLF) normalized.replace("\n","\r\n");else if(ending==Ending::CR)normalized.replace('\n','\r');
    const auto codec=e==Encoding::Utf16LE ? QStringConverter::Utf16LE : e==Encoding::Utf16BE ? QStringConverter::Utf16BE : QStringConverter::Utf8;
    QStringEncoder encoder(codec, QStringConverter::Flag::Stateless);QByteArray bytes=encoder.encode(normalized);
    if(encoder.hasError()) {*error="The document cannot be encoded without losing characters.";return {};}
    if(e==Encoding::Utf8Bom) bytes.prepend(QByteArray::fromHex("efbbbf"));
    if(e==Encoding::Utf16LE) bytes.prepend(QByteArray::fromHex("fffe"));
    if(e==Encoding::Utf16BE) bytes.prepend(QByteArray::fromHex("feff"));
    if(bytes.size()>MaxBytes) {*error="Encoded text exceeds this preview's 8 MiB limit.";return {};}
    return bytes;
}
Result load(const QString &path) {
    QFileInfo info(path);Result r;
    if(!info.isFile()) {r.error="Choose an existing regular text file.";return r;}
    QFile file(info.canonicalFilePath());if(!file.open(QIODevice::ReadOnly)){r.error=file.errorString();return r;}
    const auto bytes=file.read(MaxBytes+1);if(file.error()!=QFile::NoError){r.error=file.errorString();return r;}
    r=decode(bytes);r.file.path=info.canonicalFilePath();return r;
}
Result save(const QString &path, const QString &text, Encoding encoding, Ending ending, const QByteArray &expectedHash) {
    Result r;QFileInfo info(path);
    if(info.isSymLink()) {r.error="Saving through a symbolic link is disabled. Open its target or save a new copy.";return r;}
    const QString absolute=info.absoluteFilePath();QLockFile lock(absolute+".familiar-notepad.lock");
    if(!lock.tryLock(0)){r.error="Another Familiar Notepad operation is saving this file. Try again.";return r;}
    auto matches=[&]() -> bool {
        QFileInfo current(absolute);
        if(!current.exists()) return expectedHash.isEmpty();
        if(expectedHash.isEmpty() || !current.isFile() || current.isSymLink() || current.size()>MaxBytes)return false;
        QFile disk(absolute);return disk.open(QIODevice::ReadOnly) && digest(disk.read(MaxBytes+1))==expectedHash;
    };
    if(!matches()){r.error="The file has changed elsewhere, was removed, or already exists. Reload it or use Save As to keep both versions.";return r;}
    QString error;auto bytes=encode(text,encoding,ending,&error);if(!error.isEmpty()){r.error=error;return r;}
    QSaveFile file(absolute);file.setDirectWriteFallback(false);
    if(!file.open(QIODevice::WriteOnly)){r.error=file.errorString();return r;}
    if(!info.exists())file.setPermissions(QFile::ReadOwner|QFile::WriteOwner);
    if(file.write(bytes)!=bytes.size()){r.error=file.errorString();file.cancelWriting();return r;}
    if(!matches()){r.error="The file changed while saving. Your version was not written; use Save As.";file.cancelWriting();return r;}
    if(!file.commit()){r.error=file.errorString();return r;}
    r.ok=true;r.file={absolute,text,digest(bytes),encoding,ending};return r;
}
}
