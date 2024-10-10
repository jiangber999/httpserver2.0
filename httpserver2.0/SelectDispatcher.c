#include "EventLoop.h"
#include "Channel.h"
#include <sys/select.h>
#include <stdlib.h>
//���Ͳ���Ҫ���ļ���.h�ļ������ڲ���Ҫ��Щ����Ϊȫ�ֺ�����ֻ��Ҫ��Dispatcherʵ���ĺ���ָ��ָ����Щ�������ɣ����ǵ��õ���ָ�룬������API�ӿ�
//����Ϊ��̬������ֻ�е�ǰ�ļ�����ʹ��
#define Max 1024
static void* selectInit();
static int selectAdd(struct Channel* channel, struct EventLoop* evLoop);
static int selectRemove(struct Channel* channel, struct EventLoop* evLoop);
static int selectModify(struct Channel* channel, struct EventLoop* evLoop);
static int selectDispatch(struct EventLoop* evLoop, int timeout);
static int selectClear(struct EventLoop* evLoop);

struct Dispatcher SelectDispatch = {
	selectInit,
	selectAdd,
	selectRemove,
	selectModify,
	selectDispatch,
	selectClear,
};
struct  SelectData
{
	fd_set readSet;
	fd_set writeSet;
};
static void setFdSet(struct Channel* channel, struct SelectData* data);
static void clearFdSet(struct Channel* channel, struct SelectData* data);
static void setFdSet(struct Channel* channel,struct SelectData*data) {

	if (channel->events & ReadEvent) {
		FD_SET(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_SET(channel->fd, &data->writeSet);
	}
}
static void clearFdSet(struct Channel* channel, struct SelectData* data) {
	if (channel->events & ReadEvent) {
		FD_CLR(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_CLR(channel->fd, &data->writeSet);
	}
}
static void* selectInit() {
	struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));
	FD_ZERO(&data->readSet);
	FD_ZERO(&data->writeSet);
	return data;
}

static int selectAdd(struct Channel* channel, struct EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->DispatcherData;
	setFdSet(channel,data);

	return 0;
}
static int selectRemove(struct Channel* channel, struct EventLoop* evLoop) {	
	struct SelectData* data = (struct SelectData*)evLoop->DispatcherData;
	clearFdSet(channel,data);
	//ͨ��channel�ͷŶ�Ӧ��TcpConnection��Դ
	channel->destroyCallback(channel->arg);
	return 0;
}

static int selectModify(struct Channel* channel, struct EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->DispatcherData;
	setFdSet(channel,data);
	clearFdSet(channel,data);
	return 0;
}
static int selectDispatch(struct EventLoop* evLoop, int timeout) {
	struct SelectData* data = (struct SelectData*)evLoop->DispatcherData;
	struct timeval time = {};
	time.tv_sec = timeout;
	time.tv_usec = 0;
	fd_set readtemp=data->readSet;
	fd_set writetemp=data->writeSet;
	int count = select(Max,&readtemp,&writetemp,NULL,&time);
	if (count == -1) {
		perror("select");
		exit(0);
	}
	int i = 0;
	for (i = 0; i <Max ; i++) {
		if (FD_ISSET(i,&readtemp)) {
			eventActivate(evLoop,i, ReadEvent);
		}
		if (FD_ISSET(i,&writetemp)) {
			eventActivate(evLoop,i, WriteEvent);
		}
	}
	return 0;

}

static int selectClear(struct EventLoop* evLoop) {
	struct SelectData* data = (struct SelectData*)evLoop->DispatcherData;
	free(data);
	return 0;
}