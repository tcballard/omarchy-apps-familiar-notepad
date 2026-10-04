#include "editor.h"
#include <QFontDatabase>
#include <QTextCursor>
Editor::Editor(QWidget *parent) : QPlainTextEdit(parent) {
    setObjectName("paper");
    setAccessibleName("Document text");
    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont); font.setPointSize(12); setFont(font);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setFrameShape(QFrame::NoFrame);
    setViewportMargins(16, 12, 16, 12);
}
int Editor::replaceAll(const QString &needle, const QString &replacement, QTextDocument::FindFlags flags) {
    if (needle.isEmpty() || isReadOnly()) return 0;
    QTextCursor group(document()); group.beginEditBlock();
    QTextCursor cursor(document()); int count = 0;
    while (!(cursor = document()->find(needle, cursor, flags)).isNull()) {cursor.insertText(replacement); ++count;}
    group.endEditBlock(); return count;
}

QString Editor::text() const {auto result = document()->toRawText(); result.replace(QChar(0x2029), QChar('\n')); return result;}
