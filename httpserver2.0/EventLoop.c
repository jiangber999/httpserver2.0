#include "EventLoop.h"
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include "Log.h"
struct Dispatcher;
//写数据
void taskWakeup(struct EventLoop* evLoop) {
	//char msg[20] = "苏醒吧！我的爱人";
	char* msg = "苏醒吧！我的爱人";
	write(evLoop->socketpair[0], msg, strlen(msg));
}
//读数据
int readLocalMessage(void *arg) {
	struct EventLoop* evLoop = (struct EventLoop*)arg;
	char buf[256];
	read(evLoop->socketpair[1], buf, sizeof(buf));
	return 0;

}
struct EventLoop* EventLoopInitEx(const char* threadName)
{
	
	struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
	evLoop->isQuit = false;
	evLoop->dispatcher = &EpollDispatch;
	evLoop->DispatcherData = evLoop->dispatcher->init();
	pthread_mutex_init(&evLoop->mutex, NULL);
	evLoop->thread_ID = pthread_self();
	strcpy(evLoop->threadName,threadName == NULL ? "Mainthread" : threadName);
	//任务队列
	evLoop->head=evLoop->tail=NULL;
	//map
	evLoop->channelMap = ChannelMapInit(128);
	int ret=socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketpair);
	if (ret == -1) {
		perror("socketpair");
		exit(0);
	}

	//指定规则：socketpair[0]发数据，socketpair[1]接收数据
	struct Channel*channel=channelInit(evLoop->socketpair[1], ReadEvent, readLocalMessage, NULL, NULL,evLoop);
	//将任务添加到任务队列
	eventLoopAddTask(evLoop, channel, ADD);
	return evLoop;
}

struct EventLoop* EventLoopInit()
{
	return EventLoopInitEx(NULL);
}

int eventLoopRun(struct EventLoop* evLoop)
{
	assert(evLoop != NULL);
	//取出事件分发器和检查模型
	struct Dispatcher* dispatcher=evLoop->dispatcher;
	//比较线程ID是否正常
	if (evLoop->thread_ID != pthread_self()) {
		return -1;
	}
	//循环进行事件处理
	while (evLoop->isQuit != true) {
		dispatcher->dispatch(evLoop, 2);		//2s timeout
		eventLoopProcessTask(evLoop);
	}
	return 0;
}

int eventActivate(struct EventLoop* evLoop,int fd,int events)
{
	if (fd < 0 || evLoop == NULL) {
		return -1;
	}
	//取出Channel
	struct Channel* channel = evLoop->channelMap->list[fd];
	assert(fd==channel->fd);
	if ((ReadEvent & events)&&(channel->readCallback!=NULL)) {
		channel->readCallback(channel->arg);
	}
	if ((WriteEvent & events)&&(channel->writeCallback!=NULL)) {
		channel->readCallback(channel->arg);
	}
	return 0;
}

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
	//加锁，保护共享资源
	pthread_mutex_lock(&evLoop->mutex);
	//创建一个新节点
	struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
	
	node->channel = channel;
	node->type = type;
	node->next = NULL;
	//如果链表为空
	if (evLoop->head == NULL) {
		evLoop->head = evLoop->tail = node;
	}
	else {
		evLoop->tail->next = node;
		evLoop->tail = node;
	}
	pthread_mutex_unlock(&evLoop->mutex);
	//处理任务
	/*
		1.对于链表节点的添加，可能是当前线程也可能是其他线程（主线程）
			1）修改fd的事件，当前子线程发起，当前子线程处理
			2）添加的新fd，添加任务节点的操作有主线程发起
		2.节点任务的处理有子线程处理
	*/
	if (evLoop->thread_ID == pthread_self()) {
		//当前子线程
		eventLoopProcessTask(evLoop);
		
	}
	else {
		//主线程--告诉子线程处理任务队列中任务	子线程的状态：正在工作、正在阻塞
		taskWakeup(evLoop);
		
	}


	return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
	pthread_mutex_lock(&evLoop->mutex);
	//取出头结点
	struct ChannelElement* head = evLoop->head;
	while (head != NULL) {
		struct Channel* channel = head->channel;
		if (head->type == ADD) {
			//添加
			eventLoopAdd(evLoop, channel);
		}
		else if (head->type == DELETE) {
			//删除
			eventLoopRemove(evLoop, channel);
		}
		else if (head->type==MODIFY) {
			//修改
			eventLoopModify(evLoop, channel);
		}
		struct ChannelElement* temp = head;
		head = head->next;
		free(temp);
		//Debug("this is eventloopprocesstask");
	}
	evLoop->head = evLoop->tail = NULL;
	pthread_mutex_unlock(&evLoop->mutex);
	return 0;
}

int eventLoopAdd(struct EventLoop* evLoop,struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size) {
		//没有足够的空间存储键值对fd - channel ==>扩容
		if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*))) {
			
			return -1;
			
		}
	}
	//找到fd对应的数组元素位置，并存储
	if (channelMap->list[fd] == NULL) {
		channelMap->list[fd] = channel;
		evLoop->dispatcher->add(channel, evLoop);
	}

	return 0;
}

int eventLoopRemove(struct EventLoop* evLoop,struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size) {
		return -1;
	}

	int ret=evLoop->dispatcher->remove(channel, evLoop);
	return ret;
}

int eventLoopModify(struct EventLoop* evLoop,struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	//找到fd对应的数组元素位置
	if (channelMap->list[fd] == NULL) {
		return -1;
	}
	
	int ret=evLoop->dispatcher->modify(channel, evLoop);
	return ret;
}

int destroyChannel(struct EventLoop* evLoop,struct Channel* channel)
{
	//删除channel和fd对应关系
	evLoop->channelMap->list[channel->fd] = NULL;
	//关闭fd
	close(channel->fd);
	free(channel);
	return 0;
}
