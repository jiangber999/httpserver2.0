#pragma once
#include "EventLoop.h"
#include "Channel.h"
struct EventLoop;
struct Dispatcher {
	//��ʼ��epoll��poll��select��Ҫ�����ݿ�
	void* (*init)();
	//����ļ�������
	int (*add)(struct Channel* channel, struct EventLoop* evLoop);
	//ɾ���ļ�������
	int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
	//�޸��ļ�������
	int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
	//����¼�
	int (*dispatch)(struct EventLoop* evLoop,int timeout);//��λs
	//������ݣ��ر��ļ������������ͷ��ڴ棩
	int (*clear)(struct EventLoop* evLoop);
};