#pragma once
#include <QPlainTextEdit>
class Editor : public QPlainTextEdit {
public:
    explicit Editor(QWidget *parent = nullptr);
    QString text() const;
    int replaceAll(const QString &needle, const QString &replacement, QTextDocument::FindFlags flags = {});
    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return numbersVisible_; }
    void paintNumbers(QPaintEvent *event);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    QWidget *numbers_;
    bool numbersVisible_ = true;
    int marginWidth() const;
    void updateMargins();
};
