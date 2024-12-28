#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>

class TaskManager;

class WorkerThread : public QThread
{
    Q_OBJECT
    void run() override;
private:
    TaskManager *tkm;

public:
    WorkerThread(TaskManager *_tkm);
    ~WorkerThread();
};

#endif // WORKERTHREAD_H
