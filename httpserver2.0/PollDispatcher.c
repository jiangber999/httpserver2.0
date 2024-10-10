#include "EventLoop.h"
#include "Channel.h"
#include <poll.h>
#include <stdlib.h>
//���Ͳ���Ҫ���ļ���.h�ļ������ڲ���Ҫ��Щ����Ϊȫ�ֺ�����ֻ��Ҫ��Dispatcherʵ���ĺ���ָ��ָ����Щ�������ɣ����ǵ��õ���ָ�룬������API�ӿ�
//����Ϊ��̬������ֻ�е�ǰ�ļ�����ʹ��
#define Max 1024
static void* pollInit();
static int pollAdd(struct Channel* channel, struct EventLoop* evLoop);
static int pollRemove(struct Channel* channel, struct EventLoop* evLoop);
static int pollModify(struct Channel* channel, struct EventLoop* evLoop);
static int pollDispatch(struct EventLoop* evLoop, int timeout);
static int pollClear(struct EventLoop* evLoop);

struct Dispatcher PollDispatch = {
	pollInit,
	pollAdd,
	pollRemove,
	pollModify,
	pollDispatch,
	pollClear,
};
struct  PollData
{
	int maxfd;
	struct pollfd fds[Max];
};

static void* pollInit() {
	struct PollData* data=(struct PollData*)malloc(sizeof(struct PollData));
	data->maxfd = 0;
	int i = 0;
	for (i = 0; i < Max; i++) {
		data->fds->fd = -1;
		data->fds->events = 0;
		data->fds->revents = 0;
	}
	return data;
}

static int pollAdd(struct Channel* channel, struct EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->DispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < Max;i++) {
		if (data->fds[i].fd==-1) {
			data->fds[i].fd = channel->fd;
			data->fds[i].events = events;
			data->maxfd = data->maxfd < i ? i : data->maxfd;
			break;
		}
	}
	if (i >= Max) {
		return -1;
	}
	return 0;
}
static int pollRemove(struct Channel* channel, struct EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->DispatcherData;
	int i = 0;
	for (; i < Max; i++) {
		if (data->fds[i].fd == channel->fd) {
			data->fds[i].fd = -1;
			data->fds[i].revents = 0;
			data->fds[i].events = 0;

			break;
		}
	}
	//ͨ��channel�ͷŶ�Ӧ��TcpConnection��Դ
	channel->destroyCallback(channel->arg);
	if (i >= Max) {
		return -1;
	}
	return 0;
}

static int pollModify(struct Channel* channel, struct EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->DispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN;
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT;
	}
	int i = 0;
	for (; i < Max; i++) {
		if (data->fds[i].fd == channel->fd) {
			data->fds[i].events = events;
			break;
		}
	}
	return 0;
}
static int pollDispatch(struct EventLoop* evLoop, int timeout) {
	struct PollData* data = (struct PollData*)evLoop->DispatcherData;
	int count = poll(data->fds,data->maxfd+1,timeout*1000);
	if (count == -1) {
		perror("poll");
		exit(0);
	}
	int i = 0;
	for (i = 0; i <= data->maxfd; i++) {
		if (data->fds[i].fd == -1)continue;
		int events = data->fds[i].revents;
		if (events & POLLIN) {
			eventActivate(evLoop, data->fds[i].fd, ReadEvent);
		}
		if (events & POLLOUT) {
			eventActivate(evLoop, data->fds[i].fd, WriteEvent);
		}
	}
	return 0;

}

static int pollClear(struct EventLoop* evLoop) {
	struct PollData* data = (struct PollData*)evLoop->DispatcherData;
	
	free(data);
	return 0;
}