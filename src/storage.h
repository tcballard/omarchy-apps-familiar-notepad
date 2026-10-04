#pragma once
#include <QString>
#include <QByteArray>
namespace Note {
constexpr qint64 MaxBytes = 8 * 1024 * 1024;
enum class Encoding { Utf8, Utf8Bom, Utf16LE, Utf16BE };
enum class Ending { LF, CRLF, CR };
struct File {
    QString path, text;
    QByteArray hash;
    Encoding encoding = Encoding::Utf8;
    Ending ending = Ending::LF;
};
struct Result { bool ok = false; File file; QString error; };
QString encodingName(Encoding e);
QString endingName(Ending e);
QByteArray digest(const QByteArray &bytes);
Result decode(const QByteArray &bytes);
QByteArray encode(const QString &text, Encoding e, Ending ending, QString *error);
Result load(const QString &path);
// expectedHash empty means create only; a mismatch never overwrites.
Result save(const QString &path, const QString &text, Encoding encoding, Ending ending, const QByteArray &expectedHash);
}
