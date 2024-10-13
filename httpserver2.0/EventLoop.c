#include "EventLoop.h"
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include "Log.h"
struct Dispatcher;
//д����
void taskWakeup(struct EventLoop* evLoop) {
	//char msg[20] = "���Ѱɣ��ҵİ���";
	char* msg = "���Ѱɣ��ҵİ���";
	write(evLoop->socketpair[0], msg, strlen(msg));
}
//������
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
	//�������
	evLoop->head=evLoop->tail=NULL;
	//map
	evLoop->channelMap = ChannelMapInit(128);
	int ret=socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketpair);
	if (ret == -1) {
		perror("socketpair");
		exit(0);
	}

	//ָ������socketpair[0]�����ݣ�socketpair[1]��������
	struct Channel*channel=channelInit(evLoop->socketpair[1], ReadEvent, readLocalMessage, NULL, NULL,evLoop);
	//��������ӵ��������
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
	//ȡ���¼��ַ����ͼ��ģ��
	struct Dispatcher* dispatcher=evLoop->dispatcher;
	//�Ƚ��߳�ID�Ƿ�����
	if (evLoop->thread_ID != pthread_self()) {
		return -1;
	}
	//ѭ�������¼�����
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
	//ȡ��Channel
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
	//����������������Դ
	pthread_mutex_lock(&evLoop->mutex);
	//����һ���½ڵ�
	struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
	
	node->channel = channel;
	node->type = type;
	node->next = NULL;
	//�������Ϊ��
	if (evLoop->head == NULL) {
		evLoop->head = evLoop->tail = node;
	}
	else {
		evLoop->tail->next = node;
		evLoop->tail = node;
	}
	pthread_mutex_unlock(&evLoop->mutex);
	//��������
	/*
		1.��������ڵ����ӣ������ǵ�ǰ�߳�Ҳ�����������̣߳����̣߳�
			1���޸�fd���¼�����ǰ���̷߳��𣬵�ǰ���̴߳���
			2����ӵ���fd���������ڵ�Ĳ��������̷߳���
		2.�ڵ�����Ĵ��������̴߳���
	*/
	if (evLoop->thread_ID == pthread_self()) {
		//��ǰ���߳�
		eventLoopProcessTask(evLoop);
		
	}
	else {
		//���߳�--�������̴߳����������������	���̵߳�״̬�����ڹ�������������
		taskWakeup(evLoop);
		
	}


	return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
	pthread_mutex_lock(&evLoop->mutex);
	//ȡ��ͷ���
	struct ChannelElement* head = evLoop->head;
	while (head != NULL) {
		struct Channel* channel = head->channel;
		if (head->type == ADD) {
			//���
			eventLoopAdd(evLoop, channel);
		}
		else if (head->type == DELETE) {
			//ɾ��
			eventLoopRemove(evLoop, channel);
		}
		else if (head->type==MODIFY) {
			//�޸�
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
		//û���㹻�Ŀռ�洢��ֵ��fd - channel ==>����
		if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*))) {
			
			return -1;
			
		}
	}
	//�ҵ�fd��Ӧ������Ԫ��λ�ã����洢
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
	//�ҵ�fd��Ӧ������Ԫ��λ��
	if (channelMap->list[fd] == NULL) {
		return -1;
	}
	
	int ret=evLoop->dispatcher->modify(channel, evLoop);
	return ret;
}

int destroyChannel(struct EventLoop* evLoop,struct Channel* channel)
{
	//ɾ��channel��fd��Ӧ��ϵ
	evLoop->channelMap->list[channel->fd] = NULL;
	//�ر�fd
	close(channel->fd);
	free(channel);
	return 0;
}
