#include "editor.h"
#include <QFontDatabase>
#include <QTextCursor>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QEvent>

class NumberMargin : public QWidget {
public:
    explicit NumberMargin(Editor *editor) : QWidget(editor), editor_(editor) {setAccessibleName("Line numbers");}
protected:
    void paintEvent(QPaintEvent *event) override {editor_->paintNumbers(event);}
private:
    Editor *editor_;
};
Editor::Editor(QWidget *parent) : QPlainTextEdit(parent), numbers_(new NumberMargin(this)) {
    setObjectName("paper");
    setAccessibleName("Document text");
    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont); font.setPointSize(12); setFont(font);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setFrameShape(QFrame::NoFrame);
    document()->setDocumentMargin(18);
    connect(this, &QPlainTextEdit::blockCountChanged, this, [this]{updateMargins();});
    connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect &, int){numbers_->update();});
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this]{numbers_->update();});
    updateMargins();
}
int Editor::replaceAll(const QString &needle, const QString &replacement, QTextDocument::FindFlags flags) {
    if (needle.isEmpty() || isReadOnly()) return 0;
    QTextCursor group(document()); group.beginEditBlock();
    QTextCursor cursor(document()); int count = 0;
    while (!(cursor = document()->find(needle, cursor, flags)).isNull()) {cursor.insertText(replacement); ++count;}
    group.endEditBlock(); return count;
}

QString Editor::text() const {auto result = document()->toRawText(); result.replace(QChar(0x2029), QChar('\n')); return result;}

int Editor::marginWidth() const {
    int digits = QString::number(qMax(1, blockCount())).size();
    return numbersVisible_ ? 24 + fontMetrics().horizontalAdvance('9') * qMax(2, digits) : 0;
}
void Editor::updateMargins() {
    const int width = marginWidth();
    setViewportMargins(width, 0, 0, 0);
    numbers_->setVisible(numbersVisible_);
    numbers_->setGeometry(contentsRect().left(), contentsRect().top(), width, contentsRect().height());
    numbers_->update();
}
void Editor::setLineNumbersVisible(bool visible) {numbersVisible_ = visible; updateMargins();}
void Editor::resizeEvent(QResizeEvent *event) {QPlainTextEdit::resizeEvent(event); updateMargins();}
void Editor::changeEvent(QEvent *event) {
    QPlainTextEdit::changeEvent(event);
    if (event->type() == QEvent::FontChange) {setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4); updateMargins();}
}
void Editor::paintNumbers(QPaintEvent *event) {
    QPainter painter(numbers_);
    painter.fillRect(event->rect(), QColor("#eeece4"));
    painter.setPen(QColor("#d8d8ce"));
    painter.drawLine(numbers_->width()-1, event->rect().top(), numbers_->width()-1, event->rect().bottom());
    auto block = firstVisibleBlock();
    int number = block.blockNumber();
    qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
    while (block.isValid() && top <= event->rect().bottom()) {
        const qreal height = blockBoundingRect(block).height();
        if (block.isVisible() && top + height >= event->rect().top()) {
            const bool active = number == textCursor().blockNumber();
            if(active) {painter.fillRect(0, qRound(top), numbers_->width()-1, fontMetrics().height(), QColor("#dbe4dc"));painter.fillRect(0,qRound(top),3,fontMetrics().height(),QColor("#456357"));}
            painter.setPen(QColor(active ? "#294c3e" : "#71766f"));
            painter.drawText(0,qRound(top),numbers_->width()-12,fontMetrics().height(),Qt::AlignRight,QString::number(number+1));
        }
        top += height; block = block.next(); ++number;
    }
}
