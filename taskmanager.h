#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QMutex>
class Task;

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    //add files to calculate checksum
    void addFiles(const QStringList &files);

    //total count of checksum tasks(one file one task)
    int  totalTasks();

    //count of finshied task
    int  finishedTasks();

    //progresss of current calculating file
    double fileProgress();

    //progresss of total files that need to calculate
    double totalProgress();

    //abort a session of tasks
    void abortTasks();

    //check if task is runing
    bool isTasksRuning();

signals:
    void tasksStarted();

    void fileTaskStarted(const QFileInfo file);
    void fileTaskFinished(const QFileInfo file,const QString result);

    void tasksProgress(double fileProgress,double totalProgress,
                       int currentIndex,int totalCount);
    void tasksFinished();

public slots:
    void setCRC32(bool enabled);
    void setMD5(bool enabled);
    void setSHA1(bool enabled);
    void setSHA256(bool enabled);
    void setSHA512(bool enabled);
    void setSHA3_256(bool enabled);
    void setSHA3_512(bool enabled);
    void setKECCAK_256(bool enabled);
    void setKECCAK_512(bool enabled);
    void setDigestFormat(int format);
    void doTasks();

private:
    QList<Task*> _tasks;
    int _finishedTasks; //finished task count

    qint64 _fProgSize;    //file finshed size
    qint64 _tProgSize;    //total finished size
    qint64 _totalSize;    //total size of tasks
    bool _md5;          //can md5
    bool _sha1;         //can sha1
    bool _sha256;       //can sha256
    bool _crc32;        //can crc32
    bool _sha512;
    bool _sha3_256;
    bool _sha3_512;
    bool _keccak_256;
    bool _keccak_512;
    bool _runing;       //runing flag
    bool _abort;        //flag to abort tasks
    int _format;
    QMutex _mutex;

    void cleanTasks();
    void handleTasks();
};

#endif // TASKMANAGER_H
