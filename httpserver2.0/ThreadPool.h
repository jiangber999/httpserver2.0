#pragma once
#include "EventLoop.h"
#include "WorkerThread.h"
//定义线程池
struct ThreadPool {
	int threadNum;
	bool isstart;
	int index;
	struct WorkerThread* workerThreads;
	//主线程的反应堆模型
	struct EventLoop* mainLoop;
};
//初始化线程池
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);
//启动线程池
void threadPoolRun(struct ThreadPool*Pool);
//取出线程池中的某个子线程
struct EventLoop* takeWorkerEventLoop(struct ThreadPool*pool);


