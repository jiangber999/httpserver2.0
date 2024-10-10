#include "EventLoop.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
//解释不需要该文件的.h文件：由于不需要这些函数为全局函数，只需要把Dispatcher实例的函数指针指向这些函数即可，我们调用的是指针，而不是API接口
//设置为静态函数，只有当前文件可以使用
#define Max 520
static void* epollInit(); 
static int epollAdd(struct Channel* channel, struct EventLoop* evLoop);
static int epollRemove(struct Channel* channel, struct EventLoop* evLoop);
static int epollModify(struct Channel* channel, struct EventLoop* evLoop);
static int epollDispatch(struct EventLoop* evLoop, int timeout);
static int epollClear(struct EventLoop* evLoop);
static int epollCtl(struct Channel* channel, struct EventLoop* evLoop,int opt);//减少冗余的代码
struct Dispatcher EpollDispatch = {
	epollInit,
	epollAdd,
	epollRemove,
	epollModify,
	epollDispatch,
	epollClear,
}; 
struct  EpollData
{
	int epfd;
	struct epoll_event* events;
};


static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int opt) {
	struct EpollData* data = (struct EpollData*)evLoop->DispatcherData;
	struct epoll_event ev ;
	ev.data.fd = channel->fd;
	//no same
	if (channel->events & ReadEvent) {
		ev.events |= EPOLLIN;
	}
	if (channel->events & WriteEvent) {
		ev.events |= EPOLLOUT;
	}

	int ret = epoll_ctl(data->epfd, opt, channel->fd, &ev);

	return ret;

}

static void* epollInit() {
	struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
	data->epfd = epoll_create(10);
	if (data->epfd == -1) {
		perror("epoll_create");
		exit(0);
	}
	data->events = (struct epoll_event*)calloc(Max,sizeof(struct epoll_event));
	return data;

}

static int epollAdd(struct Channel* channel, struct EventLoop* evLoop) {
	
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_ADD);
	if (ret == -1) {
		perror("epollAdd");
	}
	
	return ret;
}
static int epollRemove(struct Channel* channel, struct EventLoop* evLoop) {
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_DEL);

	if (ret == -1) {
		perror("epollRemove");
	}	
	//通过channel释放对应的TcpConnection资源
	channel->destroyCallback(channel->arg);
	return ret;
}

static int epollModify(struct Channel* channel, struct EventLoop* evLoop) {
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_MOD);
	if (ret == -1) {
		perror("epollModify");
	}
	return ret;
}
static int epollDispatch(struct EventLoop* evLoop, int timeout) {
	struct EpollData* data = (struct EpollData*)evLoop->DispatcherData;
	int ret=epoll_wait(data->epfd, data->events, sizeof(data->events), timeout * 1000);
	int i = 0;
	for (i = 0; i < ret; i++) {
		int events = data->events[i].events;
		int fd = data->events[i].data.fd, ReadEvent;
		if (events & EPOLLERR || events & EPOLLHUP) {
			//表明有连接出现异常，需要关闭文件描述符
			//epollRemove()
			continue;
		}
		if (events&EPOLLIN) {
			eventActivate(evLoop,fd, ReadEvent);
		}
		if (events&EPOLLOUT) {
			eventActivate(evLoop,fd, WriteEvent);
		}
	}
}

static int epollClear(struct EventLoop* evLoop) {
	struct EpollData* data = (struct EpollData*)evLoop->DispatcherData;
	free(data->events);
	close(data->epfd);
	free(data);
	return 0;
}