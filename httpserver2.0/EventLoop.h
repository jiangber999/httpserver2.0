#pragma once
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
extern struct Dispatcher EpollDispatch;
extern struct Dispatcher PollDispatch;
extern struct Dispatcher SelectDispatch;
//����ýڵ���Channel�ķ�ʽ
enum ElemType {
	ADD,DELETE,MODIFY
};
//����������еĽڵ�
struct ChannelElement {
	int type;					//��δ���ڵ��е�Channel
	struct Channel* channel;
	struct ChannelElement* next;
};
struct EventLoop {
	bool isQuit;
	void* DispatcherData;
	struct Dispatcher* dispatcher;
	//�������
	struct ChannelElement* head,*tail;
	//map
	struct ChannelMap* channelMap;
	//�߳�id name mutex
	pthread_t thread_ID;
	char threadName[32];
	pthread_mutex_t mutex;
	int socketpair[2];

};
//��ʼ��
struct EventLoop* EventLoopInitEx(const char* threadName);
struct EventLoop* EventLoopInit();
//������Ӧ��ģ��
int eventLoopRun(struct EventLoop*evLoop);
//��������ļ�������
int eventActivate(struct EventLoop* evLoop,int fd,int events);
//��������������
int eventLoopAddTask(struct EventLoop*evLoop,struct Channel*channel,int type);
//������������е�����
int eventLoopProcessTask(struct EventLoop*evLoop);
//����dispatcher�еĽڵ�
int eventLoopAdd(struct EventLoop*evLoop,struct Channel*channel);
int eventLoopRemove(struct EventLoop*evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop*evLoop, struct Channel* channel);
//�ͷ�channel�������д����Ŀ�����ʵ�֣�
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);