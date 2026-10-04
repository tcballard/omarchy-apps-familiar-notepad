#pragma once
#include "storage.h"
#include "recovery.h"
#include <QMainWindow>
#include <QFutureWatcher>
#include <functional>
class Editor;
class QLabel;
class QLineEdit;
class QCheckBox;
class QTimer;
class QAction;
class Window : public QMainWindow {
public:
    explicit Window(bool readOnly = false);
    ~Window();
    void openPath(const QString &path, int line = 1, int column = 1);
    Editor *editor() const {return editor_;}
    bool busy() const {return busy_;}
    bool openSucceeded() const {return opened_;}
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Editor *editor_;
    QLabel *heading_, *location_, *position_, *format_, *notice_;
    QWidget *findBar_;
    QLineEdit *find_, *replacement_;
    QCheckBox *case_, *whole_;
    QTimer *draftTimer_;
    QAction *saveAction_, *wrapAction_;
    Note::File file_;
    Recovery recovery_;
    QFutureWatcher<Note::Result> watcher_;
    bool busy_ = false, readOnly_ = false, opened_ = false, allowClose_ = false;
    bool formatDirty_ = false;
    int fontSize_ = 12;
    std::function<void(const Note::Result &)> completion_;
    bool dirty() const;
    void updateStatus();
    void report(const QString &message);
    void guard(std::function<void()> next);
    void save(bool saveAs = false, std::function<void()> next = {});
    void run(std::function<Note::Result()> work, std::function<void(const Note::Result &)> done);
    void install(const Note::File &file);
    void newFile();
    void chooseOpen();
    void showFind(bool replace);
    void findNext(bool backward = false);
    void recover();
    void writeDraft();
};
