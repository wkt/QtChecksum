#ifndef QTAPP_H
#define QTAPP_H

#include <QObject>
#include <QApplication>
#include <QJsonObject>
#include <QNetworkReply>
#include <QCommandLineParser>
#include "utils.h"

class QtAppPriv;

extern const char *KEY_VER;
extern const char *KEY_DATE;
extern const char *KEY_LOG;
extern const char *KEY_URL;
extern const char* KEY_DEFAULT_PATH;


class QtApp : public QApplication
{
    Q_OBJECT
public:
    QtApp(int &argc, char **argv);
    ~QtApp();
    bool hasInstance();

    bool canMD5();
    bool canSHA1();
    bool canSHA256();
    bool canCRC32();

    bool canSHA512();
    bool canSHA3_256();
    bool canSHA3_512();
    bool canKECCAK_256();
    bool canKECCAK_512();

    bool isTasksRuning();
    int digestFormat();
    bool enableFileMenuItem();

    QString getValue(const QString &key,const QString &defalutValue);
    void setValue(const QString &key,const QString &value);
    bool getValueBool(const QString &key,bool defaultValue=false);
    void setValue(const QString &key,bool value);
    static int exec();

protected:
    virtual void onStartup();

protected slots:
    void handleNetworkReply(QNetworkReply* reply);
    void handleApplicationStateChanged(Qt::ApplicationState state);

    virtual bool event(QEvent *event) override;
    void handleNewArguments();

public slots:
    void checkUpdate();
    void openFiles(const QStringList &files);
    void abortTasks();
    void setMD5(bool on);
    void setSHA1(bool on);
    void setSHA256(bool on);
    void setCRC32(bool on);

    void setSHA512(bool enabled);
    void setSHA3_256(bool enabled);
    void setSHA3_512(bool enabled);
    void setKECCAK_256(bool enabled);
    void setKECCAK_512(bool enabled);
    void setDigestFormat(int format);
    void setEnableFileMenuItem(bool enabled);

private:
    QtAppPriv *priv;

signals:
    void onUpdateInfo(QJsonObject info);
    void quitRequested();

friend QtAppPriv;
};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<QtApp*>(QCoreApplication::instance()))

void comandline_add_options(QCommandLineParser &parser);

#define TR_ADD(trans,tr_name,path) \
    QTranslator trans; if (trans.load(QLocale(), QLatin1String((tr_name)), QLatin1String("_"), (path)))QCoreApplication::installTranslator(&trans)


#endif // QTAPP_H
