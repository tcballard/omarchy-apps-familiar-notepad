#include "window.h"
#include "theme.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QTimer>
#include <memory>
int main(int argc,char **argv){
    bool headless=false;
    for(int i=1;i<argc;++i){
        const QByteArray argument(argv[i]);
        if(argument=="--")break;
        if(argument=="--inspect"||argument=="--help"||argument=="--help-all"||argument=="--version"||argument=="-h"||argument=="-v")headless=true;
    }
    std::unique_ptr<QCoreApplication> app;if(headless)app=std::make_unique<QCoreApplication>(argc,argv);else app=std::make_unique<QApplication>(argc,argv);
    QCoreApplication::setApplicationName("familiar-notepad");QCoreApplication::setOrganizationName("Familiar");QCoreApplication::setApplicationVersion(NOTEPAD_VERSION);
    QCommandLineParser p;p.setApplicationDescription("Familiar Notepad — a native plain-text editor.");p.addHelpOption();p.addVersionOption();p.addPositionalArgument("file","Text file to open.","[file]");
    p.addOption({"inspect","Print encoding, line endings and file hash as JSON without opening a window."});p.addOption({"read-only","Open without allowing edits."});p.addOption({"line","Initial line (1-based).","number","1"});p.addOption({"column","Initial column (1-based).","number","1"});p.process(*app);
    auto args=p.positionalArguments();bool l,c;int line=p.value("line").toInt(&l),col=p.value("column").toInt(&c);if(args.size()>1||!l||!c||line<1||col<1){QTextStream(stderr)<<"Expected at most one file and positive line/column numbers.\n";return 2;}
    if(p.isSet("inspect")){if(args.size()!=1){QTextStream(stderr)<<"--inspect requires one file.\n";return 2;}auto r=Note::load(args[0]);QJsonObject j{{"ok",r.ok}};if(r.ok){j["path"]=r.file.path;j["encoding"]=Note::encodingName(r.file.encoding);j["lineEndings"]=Note::endingName(r.file.ending);j["sha256"]=QString::fromLatin1(r.file.hash);j["characters"]=r.file.text.size();j["lines"]=r.file.text.count('\n')+1;}else j["error"]=r.error;QTextStream(stdout)<<QJsonDocument(j).toJson();return r.ok?0:1;}
    auto *gui=static_cast<QApplication*>(app.get());gui->setStyle("Fusion");gui->setDesktopFileName("io.github.tcballard.FamiliarNotepad");applyTheme(gui);QTimer themeTimer;QObject::connect(&themeTimer,&QTimer::timeout,gui,[gui]{applyTheme(gui);});themeTimer.start(2500);
    Window window(p.isSet("read-only"));window.show();if(!args.isEmpty())window.openPath(args[0],line,col);return app->exec();
}
