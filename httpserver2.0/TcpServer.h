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
//初始化
struct TcpServer* tcpserverInit(unsigned int port,int threadNum);
//初始化监听
struct Listener* listenerInit(unsigned int port);
//启动服务器
void tcpServerRun(struct TcpServer* server);