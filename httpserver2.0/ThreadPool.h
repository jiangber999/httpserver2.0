#pragma once
#include "EventLoop.h"
#include "WorkerThread.h"
//�����̳߳�
struct ThreadPool {
	int threadNum;
	bool isstart;
	int index;
	struct WorkerThread* workerThreads;
	//���̵߳ķ�Ӧ��ģ��
	struct EventLoop* mainLoop;
};
//��ʼ���̳߳�
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);
//�����̳߳�
void threadPoolRun(struct ThreadPool*Pool);
//ȡ���̳߳��е�ĳ�����߳�
struct EventLoop* takeWorkerEventLoop(struct ThreadPool*pool);


