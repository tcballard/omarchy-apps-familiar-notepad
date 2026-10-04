#include "window.h"
#include "editor.h"
#include "theme.h"
#include <QApplication>
#include <QTimer>
#include <QAction>
int main(int argc,char**argv) {
    QApplication app(argc,argv);
    app.setApplicationName("familiar-notepad-preview");app.setStyle("Fusion");applyTheme(&app);
    if(argc<3)return 2;
    Window window;window.resize(qEnvironmentVariableIntValue("NOTEPAD_PREVIEW_NARROW")?640:1120,760);
    window.show();window.openPath(QString::fromLocal8Bit(argv[1]));
    QTimer poll;
    QObject::connect(&poll,&QTimer::timeout,&app,[&]{
        if(window.busy())return;
        poll.stop();
        if(!window.openSucceeded()){app.exit(1);return;}
        window.editor()->setFocus();
        if(qEnvironmentVariableIsSet("NOTEPAD_PREVIEW_DIRTY")) {
            window.editor()->moveCursor(QTextCursor::End);window.editor()->insertPlainText("\nOne more thought.");
        }
        QTimer::singleShot(100,&app,[&]{
            if(!window.grab().save(QString::fromLocal8Bit(argv[2]))){app.exit(2);return;}
            if(argc<=3){app.exit(0);return;}
            window.resize(800,600);
            QTimer::singleShot(100,&app,[&]{
                if(!window.grab().save(QString::fromLocal8Bit(argv[3]))){app.exit(2);return;}
                if(argc<=4){app.exit(0);return;}
                for(auto *a:window.findChildren<QAction*>())if(a->text()=="&Replace…")a->trigger();
                QTimer::singleShot(100,&app,[&]{app.exit(window.grab().save(QString::fromLocal8Bit(argv[4]))?0:2);});
            });
        });
    });
    poll.start(150);QTimer::singleShot(10000,&app,[&]{app.exit(3);});return app.exec();
}
