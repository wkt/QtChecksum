#include "qtapp.h"
#include <QNetworkAccessManager> 
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QSysInfo>
#include <QJsonDocument>
#include <QDebug>
#include <QBasicTimer>
#include <QFileOpenEvent>
#include <QCommandLineParser>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QLockFile>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include "taskmanager.h"
#include "mainwindow.h"
#include "qdocktile.h"
#include "utils.h"

const char *BASE_URL = "https://api.weiketing.com/qtchecksum/";
//const char *BASE_URL = "http://127.0.0.1:10097/qtchecksum/";

const char *KEY_VER = "ver";
const char *KEY_DATE = "date";
const char *KEY_LOG = "log";
const char *KEY_URL = "url";
const char* KEY_DEFAULT_PATH = "defaultPath";

static const char* KEY_MD5 = "MD5";
static const char* KEY_SHA1 = "SHA1";
static const char* KEY_SHA256 = "SHA256";
static const char* KEY_CRC32 = "CRC32";
static const char* KEY_SHA512 = "SHA512";
static const char* KEY_SHA3_256 = "SHA3-256";
static const char* KEY_SHA3_512 = "SHA3-512";
static const char* KEY_KECCAK256 = "KECCAK-256";
static const char* KEY_KECCAK512 = "KECCAK-512";
static const char* KEY_DIGEST_FOMART = "digest_fomat";

class QReplyTimeout : public QObject {

    enum HandleMethod { Abort, Close };

    QBasicTimer m_timer;
    HandleMethod m_method;

public:
    QReplyTimeout(QNetworkReply* reply, const int timeout_msec, HandleMethod method = Abort):QObject(reply), m_method(method)
    {
        Q_ASSERT(reply);
        if (reply && reply->isRunning()) {
          m_timer.start(timeout_msec, this);
          connect(reply, &QNetworkReply::finished, this, &QObject::deleteLater);
        }else{
          deleteLater();
        }
    }

    static void set(QNetworkReply* reply, const int timeout_msec, HandleMethod method = Abort) {
        new QReplyTimeout(reply, timeout_msec, method);
    }

protected:
  void timerEvent(QTimerEvent * ev)
  {
    if (!m_timer.isActive() || ev->timerId() != m_timer.timerId())
      return;
    auto reply = static_cast<QNetworkReply*>(parent());
    if (reply->isRunning()) {
      if (m_method == Close){
        reply->close();
      }else{
        reply->abort();
      }
    }
    m_timer.stop();
  }
};


static const char *LOCALE_NAME = "com.weiketing.qtchecksum.single_instance";
static const char *ARGS_END = "&&  \n";

static inline QString get_current_username()
{
    QString name = qgetenv("USER");
    if (name.isEmpty())
        name = qgetenv("USERNAME");
    return name;
}

static QString LOCK_FILE = "";
static inline const QString& get_lock_file(){
    if(LOCK_FILE.isEmpty()){
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        LOCK_FILE = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/qtchecksum."+get_current_username()+".lock";
    }
    //qDebug()<<"Lock file: "<<LOCK_FILE<<"\n";
    return LOCK_FILE;
}

static void handleFinderFiles(const QStringList &files)
{
    qApp->openFiles(files);
}

/**
 * 添加文件右键菜单
 **/
static void create_right_click_menuitem(){
#ifdef _WIN32
    QSettings reg("HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\QtChecksum",QSettings::NativeFormat);
    auto execFile = QDir::toNativeSeparators(QApplication::applicationFilePath());
    reg.setValue("icon",execFile);
    reg.setValue("MUIVerb",QObject::tr("Calculate checksum"));
    reg.setValue("command/.","\""+execFile+"\" \"%1\"");
    reg.sync();
#endif
}

static void remove_right_click_menuitem(){
#ifdef _WIN32
    QSettings reg("HKEY_CURRENT_USER\\Software\\Classes\\*\\shell",QSettings::NativeFormat);
    reg.remove("QtChecksum");
    reg.sync();
#endif
}

class QtAppPriv:public QObject
{

public:
    bool isStartUp = false;
    QStringList files;
    QLocalServer *server;
    QLockFile *lockfile;
    TaskManager *tkm;
    QSettings   *settings;
    MainWindow  *window;
    QDockTile *dockTile;
    QTranslator *translator;
    
    inline bool connectServer(QtApp *app){
        QLocalSocket socket;
        socket.connectToServer(LOCALE_NAME+get_current_username());
        bool ret = false;
        if(socket.waitForConnected(1000)){
            QStringList args = app->arguments();
            //qDebug()<<"args: "<<args;
            for(int i=0;i<args.size();i++){
                socket.write((args[i]+"\n").toUtf8());
                socket.flush();
                socket.waitForBytesWritten(500);
            }
            socket.write("&&  \n");
            socket.flush();
            socket.readAll();
            ret = true;
        }
        socket.close();
        //qDebug()<<__PRETTY_FUNCTION__<<",return";
        return ret;
    }
    
    inline bool setupServer(QtApp *app){
        server = new QLocalServer();
        server->setSocketOptions(QLocalServer::UserAccessOption);
        return server->listen(LOCALE_NAME+get_current_username());
    }
    
    inline void ensureHashEnabled(){
        if(window == nullptr)return;
        window->ensureHashEnabled();
    }

    QtAppPriv():server(nullptr)
      ,lockfile(new QLockFile(get_lock_file()))
      ,tkm(new TaskManager)
      ,settings(new QSettings())
      ,window(nullptr)
      ,dockTile(QDockTile::sharedQDockTile())
      ,translator(new QTranslator)
    {
    }
    
    ~QtAppPriv(){
        delete server;
        server = nullptr;
        if(lockfile->isLocked()){
            lockfile->unlock();
            lockfile->removeStaleLockFile();
        }
        tkm->abortTasks();
        tkm->deleteLater();
        settings->sync();
        delete settings;
        delete translator;
    }

public slots:
    void updateProgressStarted(){
        window->updateProgressStarted();
        dockTile->setProgress(0);
        dockTile->setIndicateNunber(tkm->totalTasks());
    }

    void updateProgress(double fileProgress,double totalProgress, int currentIndex,int totalCount){
        window->setProgress(fileProgress,totalProgress,currentIndex,totalCount);
        dockTile->setProgress(totalProgress);
        dockTile->setIndicateNunber(tkm->totalTasks() - tkm->finishedTasks());
    }

    void updateProgressFinished(){
        window->updateProgressFinished();
        dockTile->setIndicateNunber(0);
        dockTile->requestUserAttention();
    }

    void updateFileProgressStarted(const QFileInfo file){
        window->updateFileProgressStarted(file);
        dockTile->setIndicateNunber(tkm->totalTasks()-tkm->finishedTasks());
    }
    
    void updateFileProgressFinished(const QFileInfo file,const QString result){
        window->updateFileProgressFinished(file,result);
        dockTile->setIndicateNunber(tkm->totalTasks()-tkm->finishedTasks());
    }
};

QtApp::QtApp(int &argc, char **argv):QApplication(argc,argv),priv(new QtAppPriv())
{
    //setApplicationDisplayName(tr("QtChecksum"));
    connect(this,&QApplication::applicationStateChanged,this,&QtApp::handleApplicationStateChanged);
    //qDebug()<<"devicePixelRatio:"<<this->devicePixelRatio();
    priv->tkm->setDigestFormat(digestFormat());
}

QtApp::~QtApp()
{
    if(priv->window){
        delete priv->window;
        priv->window = nullptr;
    }
    delete priv;
}

void QtApp::checkUpdate()
{
    QNetworkAccessManager *nm = new QNetworkAccessManager();
    QString url = BASE_URL + QString("check_update");
    QNetworkRequest requestInfo((QUrl(url)));
    requestInfo.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QUrlQuery query;
    query.addQueryItem("os",QSysInfo::productType());
    query.addQueryItem("osVer",QSysInfo::productVersion());
    query.addQueryItem("kernel",QSysInfo::kernelType());
    query.addQueryItem("kernelVer",QSysInfo::kernelVersion());
    query.addQueryItem("cpuArch",QSysInfo::buildCpuArchitecture());
    query.addQueryItem("appVer",applicationVersion());
    query.addQueryItem("appName",applicationName());
    query.addQueryItem("verCode",QString::asprintf("%d",VERSION_CODE));

    QObject::connect(nm,&QNetworkAccessManager::finished,this,&QtApp::handleNetworkReply);
    QNetworkReply *reply = nm->post(requestInfo,query.toString().toUtf8());
    QObject::connect(reply,&QNetworkReply::finished,reply,&QObject::deleteLater);
    QReplyTimeout::set(reply,3000);
    //qDebug()<<QSslSocket::sslLibraryBuildVersionString()<<","<<QSslSocket::sslLibraryVersionString()<<","<<QSslSocket::supportsSsl();
}

void QtApp::handleNetworkReply(QNetworkReply* reply){
    QByteArray data = reply->readAll();
    QJsonObject json;
    if(reply->error() == QNetworkReply::NoError || true){
        if(data.size()>0 || true){
            QJsonDocument json_doc = QJsonDocument::fromJson(data);
            json = json_doc.object();
        }
    }
    emit onUpdateInfo(json);
    reply->manager()->deleteLater();
    reply->close();
}

void QtApp::handleApplicationStateChanged(Qt::ApplicationState state)
{
    //qDebug()<<__PRETTY_FUNCTION__<<",state:"<<state;
    if(! priv->isStartUp){
        onStartup();
    }
}


bool QtApp::event(QEvent *event)
{
    //qDebug()<<__PRETTY_FUNCTION__<<",event: "<<event;
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        openFiles(QStringList(openEvent->file()));
    }

    return QApplication::event(event);
}

void QtApp::openFiles(const QStringList &files)
{
    if(files.size() == 0)return;
    if(!priv->isStartUp){
        priv->files = files;
    }else{
        priv->tkm->addFiles(files);
        priv->window->show();
        priv->window->raise();
        priv->window->activateWindow();
    }
}

void QtApp::handleNewArguments()
{
    if(priv->server == nullptr)return;
    QLocalSocket *socket = priv->server->nextPendingConnection();
    //qDebug()<<__PRETTY_FUNCTION__<<",socket:"<<socket;
    QStringList args;
    QByteArray array;
    QString s;
    while(true){
        socket->waitForReadyRead(100);
        array = socket->readLine();
        if(array.size() <= 0){
            break;
        }
        s = QString::fromUtf8(array);
        if(s == ARGS_END){
            break;
        }
        s = s.remove(s.length()-1,1);
        args.append(s);
    }
    //qDebug()<<"args:"<<args;
    socket->write("\n");
    socket->flush();
    socket->close();
    QCommandLineParser parser;
    comandline_add_options(parser);
    parser.process(args);
    if(parser.isSet("quit")){
        emit quitRequested();
        return;
    }
    openFiles(parser.positionalArguments());
}

bool QtApp::hasInstance()
{
    bool has = false;
    if(priv->lockfile->tryLock(200)){
        has = priv->setupServer(this)?false:true;
        QObject::connect(priv->server,&QLocalServer::newConnection,this,&QtApp::handleNewArguments);
    }else{
        has = priv->connectServer(this);
    }
    return has;
}

void QtApp::onStartup()
{
    priv->isStartUp = true;

    MainWindow *window = new MainWindow();
    priv->window = window;

    connect(priv->tkm,&TaskManager::fileTaskStarted,priv,&QtAppPriv::updateFileProgressStarted);
    connect(priv->tkm,&TaskManager::tasksProgress,priv,&QtAppPriv::updateProgress);
    connect(priv->tkm,&TaskManager::fileTaskFinished,priv,&QtAppPriv::updateFileProgressFinished);
    connect(priv->tkm,&TaskManager::tasksStarted,priv,&QtAppPriv::updateProgressStarted);
    connect(priv->tkm,&TaskManager::tasksFinished,priv,&QtAppPriv::updateProgressFinished);
    window->show();

    if(priv->files.size()>0){
        openFiles(priv->files);
        priv->files.clear();
    }

#if defined (__APPLE__) && __APPLE__
    setupFinderService(handleFinderFiles);
#endif

    if (enableFileMenuItem()){
        create_right_click_menuitem();
    }else{
        remove_right_click_menuitem();
    }

    checkUpdate();
    priv->window->updateMenu();

}

void QtApp::setMD5(bool on)
{
    setValue(KEY_MD5,on);
    priv->tkm->setMD5(on);
    if(!on){
        priv->ensureHashEnabled();
    }
}

void QtApp::setSHA1(bool on){
    setValue(KEY_SHA1,on);
    priv->tkm->setSHA1(on);
    if(!on){
        priv->ensureHashEnabled();
    }
}

void QtApp::setSHA256(bool on){
    setValue(KEY_SHA256,on);
    priv->tkm->setSHA256(on);
    if(!on){
        priv->ensureHashEnabled();
    }
}

void QtApp::setCRC32(bool on){
    setValue(KEY_CRC32,on);
    priv->tkm->setCRC32(on);
    if(!on){
        priv->ensureHashEnabled();
    }
}

void QtApp::setSHA512(bool enabled){
    setValue(KEY_SHA512,enabled);
    priv->tkm->setSHA512(enabled);
    if(!enabled){
        priv->ensureHashEnabled();
    }
}

void QtApp::setSHA3_256(bool enabled){
    setValue(KEY_SHA3_256,enabled);
    priv->tkm->setSHA3_256(enabled);
    if(!enabled){
        priv->ensureHashEnabled();
    }
}

void QtApp::setSHA3_512(bool enabled){
    setValue(KEY_SHA3_512,enabled);
    priv->tkm->setSHA3_512(enabled);
    if(!enabled){
        priv->ensureHashEnabled();
    }
}

void QtApp::setKECCAK_256(bool enabled){
    setValue(KEY_KECCAK256,enabled);
    priv->tkm->setKECCAK_256(enabled);
    if(!enabled){
        priv->ensureHashEnabled();
    }
}

void QtApp::setKECCAK_512(bool enabled){
    setValue(KEY_KECCAK256,enabled);
    priv->tkm->setKECCAK_512(enabled);
    if(!enabled){
        priv->ensureHashEnabled();
    }
}

bool QtApp::QtApp::canMD5(){
    return priv->settings->value(KEY_MD5,QVariant::fromValue(true)).toBool();
}

bool QtApp::QtApp::canSHA1(){
    return priv->settings->value(KEY_SHA1,QVariant::fromValue(true)).toBool();
}

bool QtApp::canSHA256(){
    return priv->settings->value(KEY_SHA256,QVariant::fromValue(true)).toBool();
}

bool QtApp::canCRC32(){
    return priv->settings->value(KEY_CRC32,QVariant::fromValue(true)).toBool();
}

bool QtApp::canSHA512(){
    return priv->settings->value(KEY_SHA512,QVariant::fromValue(false)).toBool();
}

bool QtApp::canSHA3_256(){
    return priv->settings->value(KEY_SHA3_256,QVariant::fromValue(false)).toBool();
}

bool QtApp::canSHA3_512(){
    return priv->settings->value(KEY_SHA3_512,QVariant::fromValue(false)).toBool();
}

bool QtApp::canKECCAK_256(){
    return priv->settings->value(KEY_KECCAK256,QVariant::fromValue(false)).toBool();
}

bool QtApp::canKECCAK_512(){
    return priv->settings->value(KEY_KECCAK512,QVariant::fromValue(false)).toBool();
}

QString QtApp::getValue(const QString &key,const QString &defalutValue){
    return priv->settings->value(key,QVariant::fromValue(defalutValue)).toString();
}

void QtApp::setValue(const QString &key,const QString &value){
    priv->settings->setValue(key,QVariant::fromValue(value));
}

bool QtApp::getValueBool(const QString &key,bool defaultValue)
{
    return priv->settings->value(key,QVariant::fromValue(defaultValue)).toBool();
}

void QtApp::setValue(const QString &key,bool value){
    priv->settings->setValue(key,QVariant::fromValue(value));
    //qDebug()<<__PRETTY_FUNCTION__<<","<<key<<": "<<value;
}

void QtApp::abortTasks()
{
    priv->tkm->abortTasks();
}

bool QtApp::isTasksRuning()
{
    return priv->tkm->isTasksRuning();
}

int QtApp::digestFormat(){
    return priv->settings->value(KEY_DIGEST_FOMART,QVariant::fromValue(DIGEST_FORAMT_LOWER_CASE_HEX)).toInt();
}

void QtApp::setDigestFormat(int format){
    priv->settings->setValue(KEY_DIGEST_FOMART,QVariant::fromValue(format));
    //qDebug()<<__PRETTY_FUNCTION__<<":"<<format;
    priv->tkm->setDigestFormat(format);
}

void QtApp::setEnableFileMenuItem(bool enabled){
    priv->settings->setValue(KEY_ENABLE_FILE_MENU_ITEM,QVariant::fromValue(enabled));
    if(enabled){
        create_right_click_menuitem();
    }else{
        remove_right_click_menuitem();
    }
}

bool QtApp::enableFileMenuItem(){
    return priv->settings->value(KEY_ENABLE_FILE_MENU_ITEM,QVariant::fromValue(true)).toBool();
}


void comandline_add_options(QCommandLineParser &parser)
{
    parser.addOption(QCommandLineOption("quit"));
    parser.addPositionalArgument("files",QApplication::tr("Files to calculate the checksum"));
}


int QtApp::exec(){
    qApp->onStartup();
    return QApplication::exec();
}
