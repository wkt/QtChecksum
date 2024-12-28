
#include "taskmanager.h"
#include "workerthread.h"

#include <QCryptographicHash>
#include <QThread>
#include <iostream>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QMetaType>
#include "qckhash.h"
#include "utils.h"

#define MAX(a,b) (a)>(b)?(a):(b)

class Task{
public:
    QFileInfo   file;
    Task(const QString path):file(path){
    }
};



static inline bool isInTasks(QList<Task*>tasks,const QFileInfo& file){
    for(int i=0;i<tasks.size();i++){
        if(tasks[i]->file.absoluteFilePath() == file.absoluteFilePath()){
            return true;
        }
    }
    return false;
}

static inline bool isInTasks(QList<Task*>tasks,const QString& file){
    QFileInfo f(file);
    return isInTasks(tasks,f);
}

static inline void startThread(TaskManager *tkm)
{

    if(tkm->isTasksRuning()){
        return;
    }
    WorkerThread *thread = new WorkerThread(tkm);
    QObject::connect(thread,&QThread::finished,thread,&QObject::deleteLater);
    thread->start();
}

static const qint64 UPDATE_RATE = 200; //进度刷新率，单位：毫秒/次
static const qint64 BUFFER_SIZE = 4096;
static const int THREAD_DELAY_MS = 300; //线程延时，单位：毫秒，给主线程预留一个响应的时间

TaskManager::TaskManager(QObject *parent) : QObject(parent),_mutex()
{
    _md5 = false;
    _sha1 = false;
    _sha256 = false;
    _crc32 = false;

    _sha512 = false;
    _sha3_256 = false;
    _sha3_512 = false;
    _keccak_256 = false;
    _keccak_512 = false;

    _tasks = QList<Task*>();
    cleanTasks();
    _abort = false;
    qRegisterMetaType<QFileInfo>("QFileInfo");
}

TaskManager::~TaskManager()
{
    if(isTasksRuning()){
        abortTasks();
    }
    cleanTasks();
}

void TaskManager::addFiles(const QStringList &files){
    if(_abort)return;
    _mutex.lock();
    for(int i=0;i<files.size();i++){
        if(isInTasks(_tasks,files[i])){
            continue;
        }
        Task *tk = new Task(files[i]);
        _totalSize += tk->file.size();
        _tasks.append(tk);
    }
    startThread(this);
    _runing = true;
    _mutex.unlock();
}

int  TaskManager::totalTasks(){
    _mutex.lock();
    int res = _tasks.size();
    _mutex.unlock();
    return res;
}

int  TaskManager::finishedTasks(){
    return _finishedTasks;
}

double TaskManager::fileProgress(){
    return _fProgSize;
}

double TaskManager::totalProgress(){
    return _tProgSize;
}

void TaskManager::cleanTasks(){
    _finishedTasks = 0;

    _fProgSize = 0;
    _tProgSize = 0;
    _totalSize = 0;
    _runing = false;
    Task *tk;
    for(int i=0;i<_tasks.size();i++){
        tk = _tasks[i];
        delete tk;
        _tasks[i] = nullptr;
    }
    _tasks.clear();
}

void TaskManager::abortTasks(){
    _abort = true;
}

bool TaskManager::isTasksRuning()
{
    return _runing;
}

void TaskManager::setMD5(bool enabled){
    _md5 = enabled;
}

void TaskManager::setSHA1(bool enabled){
    _sha1 = enabled;
}

void TaskManager::setSHA256(bool enabled){
    _sha256 = enabled;
}

void TaskManager::setCRC32(bool enabled){
    _crc32 = enabled;
}

void TaskManager::setSHA512(bool enabled){
    _sha512 = enabled;
}

void TaskManager::setSHA3_256(bool enabled){
    _sha3_256 = enabled;
}

void TaskManager::setSHA3_512(bool enabled){
    _sha3_512 = enabled;
}

void TaskManager::setKECCAK_256(bool enabled){
    _keccak_256 = enabled;
}

void TaskManager::setKECCAK_512(bool enabled){
    _keccak_512 = enabled;
}

void TaskManager::handleTasks(){
    Task *tk;
    char buffer[BUFFER_SIZE] = {0};
    QList<QCKHash*> hashList;
    if(_crc32){
        hashList.append(new QCKHash(QCKHash::CRC32));
    }

    if(_md5){
        hashList.append(new QCKHash(QCKHash::MD5));
    }
    if(_sha1){
        hashList.append(new QCKHash(QCKHash::SHA1));
    }

    if(_sha256){
        hashList.append(new QCKHash(QCKHash::SHA256));
    }

    if(_sha512){
        hashList.append(new QCKHash(QCKHash::SHA512));
    }

    if(_sha3_256){
        hashList.append(new QCKHash(QCKHash::SHA3_256));
    }

    if(_sha3_512){
        hashList.append(new QCKHash(QCKHash::SHA3_512));
    }

    if(_keccak_256){
        hashList.append(new QCKHash(QCKHash::KECCAK_256));
    }

    if(_keccak_512){
        hashList.append(new QCKHash(QCKHash::KECCAK_512));
    }

    _tProgSize = 0;
    emit tasksStarted();
    QString hash;
    for(int i=0;true;i++){
        hash="";
        _fProgSize = 0;
        tk = 0;
        _mutex.lock();
        if (i<_tasks.size()){
            tk = _tasks[i];
        }
        if(_abort)break;
        if(tk == nullptr)break;
        _mutex.unlock();

        QFileInfo &qfi = tk->file;
        QFile qf(qfi.absoluteFilePath());

        //qDebug()<<"i="<<i<<","<<qfi<<qfi.absoluteFilePath()<<"\n";
        emit fileTaskStarted(qfi);
        emit tasksProgress(0.0,_tProgSize*1.0/_totalSize);
        qint64 n = 0;
        qint64 fSize = qfi.size();
        qint64 inSize = BUFFER_SIZE;
        qint64 msec = 0;
        if(qfi.isDir()){
            _tProgSize += fSize;
            _fProgSize += fSize;
        }else if(!qf.open(QFile::ReadOnly)){
            _tProgSize += fSize;
            _fProgSize += fSize;
            hash = "<br/><b>*** "+tr("open erors: ")+qf.errorString()+" ***</b><br/>";
            qf.close();
        }else {
            for(int ix=0;ix<hashList.length();ix++)hashList[ix]->reset();
            while((n = qf.read(buffer,inSize))>0){
                for(int ix=0;ix<hashList.length();ix++)hashList[ix]->addData(buffer,n);
                //qDebug()<<"_tProgSize:"<<_tProgSize;
                _tProgSize += n;
                _fProgSize += n;
                if(msec == 0 || QDateTime::currentMSecsSinceEpoch()-msec>=UPDATE_RATE){
                    msec = QDateTime::currentMSecsSinceEpoch();
                    emit tasksProgress(1.0*_fProgSize/fSize,_tProgSize*1.0/_totalSize);
                }
                if(_abort)break;
            }
            for(int ix=0;ix<hashList.length();ix++){
                QCKHash *ht = hashList[ix];
                QString h;
                if (DIGEST_FORAMT_BASE64 == _format){
                    h = ht->result().toBase64();
                }else{
                    h = ht->result().toHex();
                    if(DIGEST_FORAMT_UPPER_CASE_HEX == _format){
                        h = h.toUpper();
                    }else{
                        h = h.toLower();
                    }
                }
                hash += "<code>"+ ht->name() +":&nbsp;" +h+"<br/>";
            }
        }
        if(_abort){
            hash="<br/>"+tr("<big>***&nbsp;&nbsp; Checksum calculating aborted &nbsp;&nbsp;***</big><br/></code>");
        }
        hash+="<br/></code>";

        _finishedTasks++;
        emit tasksProgress(1.0*_fProgSize/qfi.size(),_tProgSize*1.0/_totalSize);
        emit fileTaskFinished(tk->file,hash);
        QThread::msleep(THREAD_DELAY_MS);
    }
    _runing = false;
    for(int ix=0;ix<hashList.length();ix++){
        delete hashList[ix];
        hashList[ix] = nullptr;
    }
    hashList.clear();
    //qDebug()<<"tryLock:"<<_mutex.tryLock()<<"\n";
    emit tasksFinished();
    _abort = false;
    QThread::msleep(THREAD_DELAY_MS);
    cleanTasks();
    _mutex.unlock();
}

void TaskManager::doTasks()
{
    handleTasks();
}

void TaskManager::setDigestFormat(int format)
{
    _format = format;
}
