#include "mainwindow.h"

#include "qtapp.h"

#include <QCommandLineOption>
#include <QLibraryInfo>
#include <QTranslator>

#include "utils.h"

int main(int argc, char *argv[])
{

    // 必须所有其它Qt代码之前调用，否则会无效
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setOrganizationName(QT_ORGANIZATION_NAME);
    QApplication::setApplicationName(QT_APPLICATION_NAME);
    QApplication::setApplicationVersion(VERSION);
    QtApp a(argc, argv);

    TR_ADD(qt_tr,"qtbase",QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    TR_ADD(app_tr,QT_APPLICATION_NAME,QLatin1String(":/translations"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("Qt Application for file checksum(md5/sha1/sha256/crc32) calculating"));
    parser.addHelpOption();
    parser.addVersionOption();
    comandline_add_options(parser);
    parser.process(a);

    if(parser.unknownOptionNames().size()>0){
        return 0;
    }
    if(a.hasInstance()){
        return 1;
    }
    if(parser.isSet("quit")){
        return 2;
    }
    a.openFiles(parser.positionalArguments());
    return a.exec();
}
