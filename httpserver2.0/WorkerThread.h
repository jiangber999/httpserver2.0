#pragma once
#include "EventLoop.h"
#include "pthread.h"
struct WorkerThread {
	pthread_t threadID;
	char name[24];					
	struct EventLoop* evLoop;		//��Ӧ��ģ��
	pthread_mutex_t mutex;			//������
	pthread_cond_t cond;				//��������
};
//��ʼ��
int workerThreadInit(struct WorkerThread* thread, int index);
//�����߳�
void workerThreadRun(struct WorkerThread* thread);