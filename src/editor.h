#pragma once
#include <QPlainTextEdit>
class Editor : public QPlainTextEdit {
public:
    explicit Editor(QWidget *parent = nullptr);
    QString text() const;
    int replaceAll(const QString &needle, const QString &replacement, QTextDocument::FindFlags flags = {});
};
