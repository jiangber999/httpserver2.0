#include "EventLoop.h"
#include "Channel.h"
#include <sys/select.h>
#include <stdlib.h>
//解释不需要该文件的.h文件：由于不需要这些函数为全局函数，只需要把Dispatcher实例的函数指针指向这些函数即可，我们调用的是指针，而不是API接口
//设置为静态函数，只有当前文件可以使用
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
	//通过channel释放对应的TcpConnection资源
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