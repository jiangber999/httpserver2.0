#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"
struct Listener {
	int fd;
	unsigned short port;
};
struct TcpServer {
	struct EventLoop* mainLoop;
	struct ThreadPool* pool;
	int threadNum;
	struct Listener* listener;
};
//��ʼ��
struct TcpServer* tcpserverInit(unsigned int port,int threadNum);
//��ʼ������
struct Listener* listenerInit(unsigned int port);
//����������
void tcpServerRun(struct TcpServer* server);