#include "storage.h"
#include "recovery.h"
#include "editor.h"
#include "window.h"
#include <QStandardPaths>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <QPushButton>
#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
class Tests:public QObject{
    Q_OBJECT
private slots:
    void initTestCase(){QStandardPaths::setTestModeEnabled(true);}
    void encodings(){const QString text=QString::fromUtf8("Hello, café 🌍\n第二行\n");for(int e=0;e<4;++e)for(int n=0;n<3;++n){QString error;auto bytes=Note::encode(text,Note::Encoding(e),Note::Ending(n),&error);QVERIFY2(error.isEmpty(),qPrintable(error));auto r=Note::decode(bytes);QVERIFY2(r.ok,qPrintable(r.error));QCOMPARE(r.file.text,text);QCOMPARE(r.file.encoding,Note::Encoding(e));QCOMPARE(r.file.ending,Note::Ending(n));QCOMPARE(Note::encode(r.file.text,r.file.encoding,r.file.ending,&error),bytes);}}
    void leadingBomCharacter(){auto bytes=QByteArray::fromHex("efbbbfefbbbf6162");auto r=Note::decode(bytes);QVERIFY(r.ok);QCOMPARE(r.file.text,QString(QChar(0xfeff))+"ab");QString err;QCOMPARE(Note::encode(r.file.text,r.file.encoding,r.file.ending,&err),bytes);}
    void unsafeInput(){QVERIFY(!Note::decode(QByteArray("a\0b",3)).ok);QVERIFY(!Note::decode("a\r\nb\n").ok);QVERIFY(!Note::decode(QByteArray::fromHex("c3")).ok);QVERIFY(!Note::decode(QByteArray::fromHex("fffefd")).ok);QVERIFY(!Note::decode(QByteArray(Note::MaxBytes+1,'x')).ok);}
    void atomicSaveAndConflict(){QTemporaryDir dir;auto p=dir.filePath("spaces in name.txt");auto first=Note::save(p,"one\n",Note::Encoding::Utf8,Note::Ending::CRLF,{});QVERIFY2(first.ok,qPrintable(first.error));auto second=Note::save(p,"two\n",Note::Encoding::Utf8,Note::Ending::CRLF,first.file.hash);QVERIFY(second.ok);QCOMPARE(Note::load(p).file.text,QString("two\n"));QVERIFY(!Note::save(p,"clobber",Note::Encoding::Utf8,Note::Ending::LF,first.file.hash).ok);QCOMPARE(Note::load(p).file.text,QString("two\n"));QVERIFY(!Note::save(p,"clobber",Note::Encoding::Utf8,Note::Ending::LF,{}).ok);QFile::remove(p);QVERIFY(!Note::save(p,"clobber",Note::Encoding::Utf8,Note::Ending::LF,second.file.hash).ok);QVERIFY(!QFile::exists(p));}
    void recovery(){QTemporaryDir dir;QString path;{Recovery active(dir.path());Note::File f;f.path="/tmp/original.txt";f.text="Unsaved café";QString error;QVERIFY(active.write(f,&error));path=active.path();Recovery other(dir.path());QVERIFY(other.available().isEmpty());}Recovery next(dir.path());QCOMPARE(next.available().size(),1);auto r=next.read(path);QVERIFY(r.ok);QCOMPARE(r.file.text,QString("Unsaved café"));QVERIFY(next.remove(path));QVERIFY(next.available().isEmpty());}
    void widgetFileFlow(){QTemporaryDir dir;auto p=dir.filePath("real flow.txt");const auto original=QString::fromUtf8("nonbreaking: a b 🌍\nlast\n");auto r=Note::save(p,original,Note::Encoding::Utf16LE,Note::Ending::CRLF,{});QVERIFY(r.ok);Window w;w.show();w.openPath(p,2,2);QTRY_VERIFY(!w.busy());QVERIFY(w.openSucceeded());QCOMPARE(w.editor()->text(),original);QCOMPARE(w.editor()->textCursor().blockNumber(),1);w.editor()->moveCursor(QTextCursor::End);QTest::keyClicks(w.editor(),"extra");QVERIFY(w.editor()->document()->isModified());QAction *save=nullptr;for(auto *a:w.findChildren<QAction*>())if(a->text()=="&Save")save=a;QVERIFY(save);save->trigger();QTRY_VERIFY(!w.busy());QVERIFY(!w.editor()->document()->isModified());auto saved=Note::load(p);QVERIFY(saved.ok);QCOMPARE(saved.file.text,original+"extra");QCOMPARE(saved.file.encoding,Note::Encoding::Utf16LE);QCOMPARE(saved.file.ending,Note::Ending::CRLF);QTest::keyClicks(w.editor()," pending");QTimer::singleShot(20,[]{auto *box=qobject_cast<QMessageBox*>(QApplication::activeModalWidget());if(box)box->button(QMessageBox::Cancel)->click();});QVERIFY(!w.close());QVERIFY(w.editor()->document()->isModified());w.editor()->document()->setModified(false);QVERIFY(w.close());}
    void readonlyInput(){Window w(true);w.show();bool found=false;for(auto *a:w.findChildren<QAction*>())if(a->text()=="&Recover drafts…"){found=true;QVERIFY(!a->isEnabled());}QVERIFY(found);QTest::keyClicks(w.editor(),"ignored");QCOMPARE(w.editor()->text(),QString());}
    void gutterAndZoom(){Editor e;e.resize(640,400);e.show();const QString sample=QString("A long wrapped line ").repeated(25)+"\nSecond line\n";e.setPlainText(sample);const int withNumbers=e.viewport()->width();e.setLineNumbersVisible(false);QVERIFY(e.viewport()->width()>withNumbers);e.setLineNumbersVisible(true);QCOMPARE(e.viewport()->width(),withNumbers);e.zoomIn(3);QCOMPARE(e.text(),sample);e.moveCursor(QTextCursor::End);e.ensureCursorVisible();QCOMPARE(e.textCursor().blockNumber(),2);e.setLineWrapMode(QPlainTextEdit::NoWrap);QCOMPARE(e.text(),sample);}
    void replacementUndo(){Editor e;e.setPlainText("cat cat scatter cat");QCOMPARE(e.replaceAll("cat","dog",QTextDocument::FindWholeWords),3);QCOMPARE(e.toPlainText(),QString("dog dog scatter dog"));e.undo();QCOMPARE(e.toPlainText(),QString("cat cat scatter cat"));QCOMPARE(e.replaceAll("cat","catcat"),4);e.undo();QCOMPARE(e.toPlainText(),QString("cat cat scatter cat"));e.setReadOnly(true);QCOMPARE(e.replaceAll("cat","dog"),0);}
};
QTEST_MAIN(Tests)
#include "tests.moc"
