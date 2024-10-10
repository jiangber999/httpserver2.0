#pragma once
#include "EventLoop.h"
#include "pthread.h"
struct WorkerThread {
	pthread_t threadID;
	char name[24];					
	struct EventLoop* evLoop;		//反应堆模型
	pthread_mutex_t mutex;			//互斥锁
	pthread_cond_t cond;				//条件变量
};
//初始化
int workerThreadInit(struct WorkerThread* thread, int index);
//启动线程
void workerThreadRun(struct WorkerThread* thread);