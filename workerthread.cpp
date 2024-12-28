#include "workerthread.h"
#include <taskmanager.h>

WorkerThread::WorkerThread(TaskManager *_tkm) : QThread(),tkm(_tkm)
{
}

void WorkerThread::run() {
    tkm->doTasks();
}

WorkerThread::~WorkerThread(){
    tkm = nullptr;
}
