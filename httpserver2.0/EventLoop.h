#pragma once
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
extern struct Dispatcher EpollDispatch;
extern struct Dispatcher PollDispatch;
extern struct Dispatcher SelectDispatch;
//处理该节点中Channel的方式
enum ElemType {
	ADD,DELETE,MODIFY
};
//定义任务队列的节点
struct ChannelElement {
	int type;					//如何处理节点中的Channel
	struct Channel* channel;
	struct ChannelElement* next;
};
struct EventLoop {
	bool isQuit;
	void* DispatcherData;
	struct Dispatcher* dispatcher;
	//任务队列
	struct ChannelElement* head,*tail;
	//map
	struct ChannelMap* channelMap;
	//线程id name mutex
	pthread_t thread_ID;
	char threadName[32];
	pthread_mutex_t mutex;
	int socketpair[2];

};
//初始化
struct EventLoop* EventLoopInitEx(const char* threadName);
struct EventLoop* EventLoopInit();
//启动反应堆模型
int eventLoopRun(struct EventLoop*evLoop);
//处理激活的文件描述符
int eventActivate(struct EventLoop* evLoop,int fd,int events);
//添加任务到任务队列
int eventLoopAddTask(struct EventLoop*evLoop,struct Channel*channel,int type);
//处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop*evLoop);
//处理dispatcher中的节点
int eventLoopAdd(struct EventLoop*evLoop,struct Channel*channel);
int eventLoopRemove(struct EventLoop*evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop*evLoop, struct Channel* channel);
//释放channel（最好是写到项目的最后实现）
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);