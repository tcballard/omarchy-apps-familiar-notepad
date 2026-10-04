#include "window.h"
#include "editor.h"
#include "icons.h"
#include <QGridLayout>
#include <QtConcurrent>
#include <QApplication>
#include <QActionGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTextBlock>
#include <QTimer>
#include <QToolBar>

Window::Window(bool readOnly) : readOnly_(readOnly) {
    resize(1120, 760); setMinimumSize(640, 400);
    setWindowIcon(QIcon(":/notepad.svg"));
    auto *body = new QWidget; auto *layout = new QVBoxLayout(body); layout->setContentsMargins(12, 12, 12, 12); layout->setSpacing(0);
    auto *header = new QWidget; header->setObjectName("documentHeader"); auto *headerLayout = new QHBoxLayout(header); headerLayout->setContentsMargins(14, 0, 14, 0); header->setFixedHeight(43);
    heading_ = new QLabel; heading_->setObjectName("documentTitle"); location_ = new QLabel; location_->setObjectName("documentPath");
    heading_->setMinimumWidth(0);heading_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);heading_->setTextFormat(Qt::PlainText); location_->setTextFormat(Qt::PlainText);
    location_->setMinimumWidth(140);location_->setAlignment(Qt::AlignRight|Qt::AlignVCenter);location_->setTextInteractionFlags(Qt::TextSelectableByMouse); auto *fileIcon = new QLabel; fileIcon->setPixmap(toolIcon("new", QColor("#e8cf87")).pixmap(20,20)); headerLayout->addWidget(fileIcon); headerLayout->addSpacing(5); headerLayout->addWidget(heading_,1); headerLayout->addWidget(location_);
    layout->addWidget(header);
    notice_ = new QLabel; notice_->setTextFormat(Qt::PlainText); notice_->setWordWrap(true); notice_->setObjectName("notice"); notice_->hide(); layout->addWidget(notice_);
    findBar_ = new QWidget; auto *search = new QGridLayout(findBar_); findBar_->setObjectName("searchPanel"); search->setContentsMargins(10, 8, 10, 8);
    find_ = new QLineEdit; find_->setPlaceholderText("Find text"); find_->setAccessibleName("Find text"); replacement_ = new QLineEdit; replacement_->setPlaceholderText("Replace with"); replacement_->setAccessibleName("Replacement text");
    case_ = new QCheckBox("Case"); whole_ = new QCheckBox("Whole word");
    search->addWidget(find_,0,0,1,2); search->addWidget(replacement_,1,0,1,2); search->addWidget(case_,2,0); search->addWidget(whole_,2,1); search->setColumnStretch(1,1);
    auto button = [search, this](QString text, auto action) {auto *b = new QPushButton(text); b->setMinimumWidth(80); connect(b, &QPushButton::clicked, this, action); return b;};
    search->addWidget(button("Previous", [this]{findNext(true);}),0,2); search->addWidget(button("Next", [this]{findNext();}),0,3);
    auto *all = button("Replace all", [this]{auto flags = QTextDocument::FindFlags{}; if(case_->isChecked()) flags |= QTextDocument::FindCaseSensitively; if(whole_->isChecked()) flags |= QTextDocument::FindWholeWords; int n = editor_->replaceAll(find_->text(), replacement_->text(), flags); statusBar()->showMessage(QString("Replaced %1 occurrence(s)").arg(n), 4000);}); all->setObjectName("replaceAll"); search->addWidget(all,1,2,1,2);
    search->addWidget(button("Close", [this]{findBar_->hide(); editor_->setFocus();}),2,3);
    layout->addWidget(findBar_); findBar_->hide();
    editor_ = new Editor; editor_->setReadOnly(readOnly_); layout->addWidget(editor_, 1); setCentralWidget(body);
    auto *file = menuBar()->addMenu("&File"); auto *edit = menuBar()->addMenu("&Edit"); auto *formatMenu = menuBar()->addMenu("F&ormat"); auto *view = menuBar()->addMenu("&View"); auto *help = menuBar()->addMenu("&Help");
    auto *bar = addToolBar("Main toolbar"); bar->setMovable(false); bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon); bar->setIconSize(QSize(20,20));
    auto add = [this](QMenu *menu, QString text, QKeySequence key, auto slot) {auto *a = menu->addAction(text); a->setShortcut(key); a->setToolTip(text.remove('&')+(key.isEmpty()?QString():"  "+key.toString(QKeySequence::NativeText))); connect(a, &QAction::triggered, this, slot); return a;};
    bar->addAction(add(file, "&New", QKeySequence::New, [this]{guard([this]{newFile();});}));
    bar->addAction(add(file, "&Open…", QKeySequence::Open, [this]{chooseOpen();}));
    saveAction_ = add(file, "&Save", QKeySequence::Save, [this]{save();}); bar->addAction(saveAction_);
    add(file, "Save &as…", QKeySequence::SaveAs, [this]{save(true);}); file->addSeparator();
    add(file, "&Recover drafts…", {}, [this]{guard([this]{recover();});})->setEnabled(!readOnly_); file->addSeparator();
    add(file, "E&xit", QKeySequence::Quit, [this]{close();});
    bar->addSeparator();
    auto *undo = add(edit, "&Undo", QKeySequence::Undo, [this]{editor_->undo();}); auto *redo = add(edit, "&Redo", QKeySequence::Redo, [this]{editor_->redo();});
    undo->setEnabled(false); redo->setEnabled(false); connect(editor_, &QPlainTextEdit::undoAvailable, undo, &QAction::setEnabled); connect(editor_, &QPlainTextEdit::redoAvailable, redo, &QAction::setEnabled); bar->addAction(undo); bar->addAction(redo);
    edit->addSeparator(); add(edit, "Cu&t", QKeySequence::Cut, [this]{editor_->cut();}); add(edit, "&Copy", QKeySequence::Copy, [this]{editor_->copy();}); add(edit, "&Paste", QKeySequence::Paste, [this]{editor_->paste();}); add(edit, "Select &all", QKeySequence::SelectAll, [this]{editor_->selectAll();});
    edit->addSeparator(); bar->addSeparator();
    bar->addAction(add(edit, "&Find…", QKeySequence::Find, [this]{showFind(false);}));
    bar->addAction(add(edit, "&Replace…", QKeySequence::Replace, [this]{showFind(true);}));
    add(edit, "Find next", QKeySequence::FindNext, [this]{findNext();});
    add(edit, "Find previous", QKeySequence::FindPrevious, [this]{findNext(true);});
    add(edit, "&Go to…", QKeySequence("Ctrl+G"), [this]{bool ok; auto input = QInputDialog::getText(this, "Go to", "Line:column", QLineEdit::Normal, "1:1", &ok); if(!ok) return; auto bits=input.split(':'); bool good; int line=bits.value(0).toInt(&good); int col=bits.size()>1?bits.value(1).toInt():1; if(!good||line<1||line>editor_->document()->blockCount()||col<1) {report("Enter an existing line and a positive column, for example 12:3."); return;} auto block=editor_->document()->findBlockByNumber(line-1); QTextCursor c(block); c.setPosition(block.position()+qMin(col-1,block.length()-1)); editor_->setTextCursor(c); editor_->setFocus();});
    wrapAction_ = add(formatMenu, "&Word wrap", {}, [this]{editor_->setLineWrapMode(wrapAction_->isChecked()?QPlainTextEdit::WidgetWidth:QPlainTextEdit::NoWrap);}); wrapAction_->setCheckable(true); wrapAction_->setChecked(true); bar->addSeparator(); bar->addAction(wrapAction_);
    add(formatMenu, "&Font…", {}, [this]{bool ok; auto f=QFontDialog::getFont(&ok,editor_->font(),this); if(ok){editor_->setFont(f);fontSize_=f.pointSize(); updateStatus();}});
    auto *enc = formatMenu->addMenu("Encoding"); auto *encGroup = new QActionGroup(this);
    for (int i=0;i<4;++i) {auto value=Note::Encoding(i);auto *a=enc->addAction(Note::encodingName(value));a->setCheckable(true);a->setData(i);encGroup->addAction(a); connect(a,&QAction::triggered,this,[this,value]{if(readOnly_||busy_)return; file_.encoding=value;formatDirty_=true;updateStatus();draftTimer_->start();});}
    connect(enc,&QMenu::aboutToShow,this,[this,encGroup]{for(auto *a:encGroup->actions()) a->setChecked(a->data().toInt()==int(file_.encoding));});
    auto *eol = formatMenu->addMenu("Line endings"); auto *eolGroup=new QActionGroup(this);
    for(int i=0;i<3;++i){auto value=Note::Ending(i);auto *a=eol->addAction(Note::endingName(value));a->setCheckable(true);a->setData(i);eolGroup->addAction(a);connect(a,&QAction::triggered,this,[this,value]{if(readOnly_||busy_)return;file_.ending=value;formatDirty_=true;updateStatus();draftTimer_->start();});}
    connect(eol,&QMenu::aboutToShow,this,[this,eolGroup]{for(auto *a:eolGroup->actions())a->setChecked(a->data().toInt()==int(file_.ending));});
    add(view,"Zoom &in",QKeySequence::ZoomIn,[this]{if(editor_->font().pointSize()<40)editor_->zoomIn();updateStatus();}); add(view,"Zoom &out",QKeySequence::ZoomOut,[this]{if(editor_->font().pointSize()>8)editor_->zoomOut();updateStatus();}); add(view,"&Reset zoom",QKeySequence("Ctrl+0"),[this]{auto f=editor_->font();f.setPointSize(fontSize_);editor_->setFont(f);updateStatus();});
    auto *lines = add(view, "&Line numbers", {}, [this]{editor_->setLineNumbersVisible(!editor_->lineNumbersVisible());}); lines->setCheckable(true);lines->setChecked(true);
    const QStringList glyphs{"new","open","save","undo","redo","find","replace","wrap"}; int glyphIndex=0;
    for(auto *action:bar->actions())if(!action->isSeparator()){const auto glyph=glyphs.value(glyphIndex++);action->setProperty("familiarGlyph",glyph);action->setIcon(toolIcon(glyph,palette().color(QPalette::ButtonText)));}
    add(help,"&About Familiar Notepad",{},[this]{QMessageBox::about(this,"Familiar Notepad",QString("Familiar Notepad %1\n\nA little space for plain text.\nNative Qt. Local files. No account.\n\nMIT licence · Tom Ballard").arg(NOTEPAD_VERSION));});
    position_=new QLabel;format_=new QLabel;statusBar()->addWidget(position_,1);statusBar()->addPermanentWidget(format_);
    draftTimer_=new QTimer(this);draftTimer_->setSingleShot(true);draftTimer_->setInterval(1000);connect(draftTimer_,&QTimer::timeout,this,[this]{writeDraft();});
    connect(editor_,&QPlainTextEdit::textChanged,this,[this]{updateStatus();if(dirty())draftTimer_->start();}); connect(editor_,&QPlainTextEdit::cursorPositionChanged,this,[this]{updateStatus();}); connect(editor_->document(),&QTextDocument::modificationChanged,this,[this]{updateStatus();}); connect(find_,&QLineEdit::returnPressed,this,[this]{findNext();});
    auto *escape=new QAction(this);escape->setShortcut(QKeySequence(Qt::Key_Escape));addAction(escape);connect(escape,&QAction::triggered,this,[this]{findBar_->hide();editor_->setFocus();});
    connect(&watcher_,&QFutureWatcher<Note::Result>::finished,this,[this]{busy_=false;editor_->setReadOnly(readOnly_);menuBar()->setEnabled(true); for(auto *bar:findChildren<QToolBar*>())bar->setEnabled(true);auto done=std::move(completion_);auto result=watcher_.result();updateStatus();if(done)done(result);});
    newFile();
    if(!recovery_.available().isEmpty()){notice_->setText("A recovery draft is available. Open File → Recover drafts to restore it.");notice_->show();}
}
Window::~Window(){watcher_.waitForFinished();}
bool Window::dirty()const{return editor_->document()->isModified()||formatDirty_;}
void Window::updateStatus(){
    const auto name=file_.path.isEmpty()?QString("Untitled"):QFileInfo(file_.path).fileName();
    setWindowTitle(QString("%1%2 — Familiar Notepad%3").arg(dirty()?"* ":"",name,readOnly_?" [read only]":""));
    heading_->setText(QFontMetrics(heading_->font()).elidedText(name,Qt::ElideMiddle,qMax(160,width()-230))); heading_->setToolTip(file_.path);location_->setText(busy_?"Working…":readOnly_?"Read only":dirty()?"●  Unsaved changes":file_.path.isEmpty()?"New document":"✓  Saved");location_->setToolTip(file_.path);
    auto c=editor_->textCursor();position_->setText(QString("Ln %1, Col %2   ·   %3 characters").arg(c.blockNumber()+1).arg(c.positionInBlock()+1).arg(editor_->document()->characterCount()-1));
    format_->setText(QString("%1   ·   %2   ·   %3%").arg(Note::encodingName(file_.encoding),Note::endingName(file_.ending)).arg(qRound(editor_->font().pointSizeF()/fontSize_*100)));
    saveAction_->setEnabled(!readOnly_&&!busy_);
}
void Window::report(const QString &message){QMessageBox::warning(this,"Familiar Notepad",message);}
void Window::run(std::function<Note::Result()> work,std::function<void(const Note::Result&)> done){if(busy_)return;busy_=true;completion_=std::move(done);editor_->setReadOnly(true);updateStatus();menuBar()->setEnabled(false);for(auto *bar:findChildren<QToolBar*>())bar->setEnabled(false);watcher_.setFuture(QtConcurrent::run(std::move(work)));}
void Window::install(const Note::File &file){file_=file;editor_->setPlainText(file.text);editor_->document()->setModified(false);formatDirty_=false;draftTimer_->stop();recovery_.clear();notice_->hide();updateStatus();}
void Window::newFile(){install({});editor_->setFocus();}
void Window::guard(std::function<void()> next){if(busy_)return;if(!dirty()){next();return;}auto choice=QMessageBox::question(this,"Save changes?","Save changes to this document before continuing?",QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel,QMessageBox::Save);if(choice==QMessageBox::Save)save(false,std::move(next));else if(choice==QMessageBox::Discard){next();}}
void Window::chooseOpen(){if(busy_)return;auto path=QFileDialog::getOpenFileName(this,"Open text file",file_.path,"Text files (*.txt *.md *.log *.csv *.json *.toml *.yaml *.yml *.ini *.conf);;All files (*)");if(!path.isEmpty())guard([this,path]{openPath(path);});}
void Window::openPath(const QString &path,int line,int column){if(busy_)return;opened_=false;run([path]{return Note::load(path);},[this,line,column](const Note::Result&r){if(!r.ok){report(r.error);return;}install(r.file);opened_=true;auto block=editor_->document()->findBlockByNumber(qBound(0,line-1,editor_->document()->blockCount()-1));QTextCursor c(block);c.setPosition(block.position()+qBound(0,column-1,block.length()-1));editor_->setTextCursor(c);editor_->setFocus();});}
void Window::save(bool saveAs,std::function<void()> next){
    if(busy_||readOnly_)return;QString path=file_.path;QByteArray expected=file_.hash;
    if(saveAs||path.isEmpty()){
        path=QFileDialog::getSaveFileName(this,"Save text file",path.isEmpty()?"Untitled.txt":path,"Text files (*.txt);;All files (*)",nullptr,QFileDialog::DontConfirmOverwrite);
        if(path.isEmpty())return;expected.clear();
        if(QFileInfo::exists(path)){auto prior=Note::load(path);if(!prior.ok){report(prior.error);return;} expected=prior.file.hash;if(QMessageBox::question(this,"Replace existing file?","Replace “"+QFileInfo(path).fileName()+"” with this document?",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;}
    }
    auto text=editor_->text();auto encoding=file_.encoding;auto ending=file_.ending;
    run([path,text,encoding,ending,expected]{return Note::save(path,text,encoding,ending,expected);},[this,next](const Note::Result&r){if(!r.ok){report(r.error+"\n\nYour edits remain open. Use Save as to keep a separate copy.");return;}file_=r.file;editor_->document()->setModified(false);formatDirty_=false;draftTimer_->stop();recovery_.clear();notice_->hide();updateStatus();statusBar()->showMessage("Saved",2500);if(next)next();});
}
void Window::showFind(bool replace){replacement_->setVisible(replace);findBar_->findChild<QPushButton*>("replaceAll")->setVisible(replace);findBar_->show();if(editor_->textCursor().hasSelection())find_->setText(editor_->textCursor().selectedText());find_->setFocus();find_->selectAll();}
void Window::findNext(bool backward){if(find_->text().isEmpty()){showFind(false);return;}QTextDocument::FindFlags flags;if(case_->isChecked())flags|=QTextDocument::FindCaseSensitively;if(whole_->isChecked())flags|=QTextDocument::FindWholeWords;if(backward)flags|=QTextDocument::FindBackward;auto old=editor_->textCursor();if(!editor_->find(find_->text(),flags)){auto c=old;c.movePosition(backward?QTextCursor::End:QTextCursor::Start);editor_->setTextCursor(c);if(!editor_->find(find_->text(),flags)){editor_->setTextCursor(old);statusBar()->showMessage("Text not found",3000);}}}
void Window::writeDraft(){if(!dirty()||readOnly_)return;auto draft=file_;draft.text=editor_->text();QString error;if(!recovery_.write(draft,&error)){notice_->setText("Recovery draft could not be saved: "+error);notice_->show();}}
void Window::recover(){auto paths=recovery_.available();if(paths.isEmpty()){statusBar()->showMessage("No recovery drafts available",3000);return;}QStringList labels;for(const auto&p:paths){auto r=recovery_.read(p);labels<<(r.ok?(r.file.path.isEmpty()?"Untitled":r.file.path):"Unreadable draft")+" — "+QFileInfo(p).lastModified().toString("yyyy-MM-dd HH:mm:ss")+" ["+QFileInfo(p).baseName().left(8)+"]";}bool ok;auto chosen=QInputDialog::getItem(this,"Recover a draft","Restore edits without overwriting the original file:",labels,0,false,&ok);if(!ok)return;int index=labels.indexOf(chosen);auto r=recovery_.read(paths[index]);if(!r.ok){report(r.error);return;}install(r.file);editor_->document()->setModified(true);QString error;auto draft=file_;draft.text=editor_->text();if(recovery_.write(draft,&error))recovery_.remove(paths[index]);else report("Recovered text is open, but a new recovery snapshot could not be saved: "+error);
    notice_->setText("Recovered draft. Review it, then Save or Save as. The original file has not been changed.");notice_->show();}
void Window::closeEvent(QCloseEvent *event){if(allowClose_||(!busy_&&!dirty())){draftTimer_->stop();recovery_.clear();event->accept();return;}event->ignore();if(busy_){statusBar()->showMessage("Please wait for the file operation to finish.",3000);return;}guard([this]{draftTimer_->stop();recovery_.clear();allowClose_=true;QTimer::singleShot(0,this,[this]{close();});});}

void Window::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if(event->type()==QEvent::PaletteChange)for(auto *action:findChildren<QAction*>()){const auto glyph=action->property("familiarGlyph").toString();if(!glyph.isEmpty())action->setIcon(toolIcon(glyph,palette().color(QPalette::ButtonText)));}
}
