#include "window.h"
#include "editor.h"
#include "theme.h"
#include <QApplication>
#include <QTimer>
#include <QFileInfo>
#include <QAction>
int main(int argc,char**argv){QApplication app(argc,argv);app.setApplicationName("familiar-notepad-preview");app.setStyle("Fusion");applyTheme(&app);if(argc<3)return 2;Window window;window.resize(1120,760);window.show();window.openPath(QString::fromLocal8Bit(argv[1]));QTimer poll;QObject::connect(&poll,&QTimer::timeout,&app,[&]{if(window.busy())return;if(!window.openSucceeded()){app.exit(1);return;}window.editor()->setFocus();window.grab().save(QString::fromLocal8Bit(argv[2]));if(argc>3){window.resize(800,600);QTimer::singleShot(100,&app,[&]{window.grab().save(QString::fromLocal8Bit(argv[3]));if(argc>4){for(auto *a:window.findChildren<QAction*>())if(a->text()=="&Replace…")a->trigger();QTimer::singleShot(100,&app,[&]{window.grab().save(QString::fromLocal8Bit(argv[4]));app.quit();});}else app.quit();});poll.stop();}else app.quit();});poll.start(150);QTimer::singleShot(10000,&app,[&]{app.exit(3);});return app.exec();}
