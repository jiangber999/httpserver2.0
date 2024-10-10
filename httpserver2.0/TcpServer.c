#include "TcpServer.h"
#include <sys/socket.h>
#include "Channel.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdlib.h>
struct TcpServer* tcpserverInit(unsigned int port, int threadNum)
{
	struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
	tcp->mainLoop = EventLoopInit();
	tcp->threadNum = threadNum;
	tcp->pool = threadPoolInit(tcp->mainLoop, tcp->threadNum);
	tcp->listener = listenerInit(port);
	return tcp;
}

struct Listener* listenerInit(unsigned int port)
{
	//创建监听套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("Socket error");
		return NULL;
	}
	//设置端口复用
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret == -1) {
		perror("Setsockopt error");
		return NULL;
	}
	//绑定地址结构
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);//可以不转换 大端0和小端0是一样的
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		perror("Bind error");
		return NULL;
	}
	//设置与服务器同时建立连接的数量
	ret = listen(lfd, 128);
	if (ret == -1) {
		perror("Listen error");
		return NULL;
	}

	
	struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
	listener->fd = lfd;
	listener->port = port;
	return listener;
}

//监听文件描述符回调函数
int acceptConnection(void*arg) {
	struct TcpServer* server = (struct TcpServer*)arg;
	//和客户端建立连接
	int cfd = accept(server->listener->fd, NULL, NULL);
	//从线程池中取出一个子线程的反应堆实例，去处理这个lfd
	struct EventLoop*evLoop=takeWorkerEventLoop(server->pool);
	//将cfd放到TcpConnection中处理
	TcpConnectionInit(evLoop,cfd);

}
void tcpServerRun(struct TcpServer* server)
{
	//启动线程池;
	threadPoolRun(server->pool);
	//添加检测的任务
	struct Channel* channel = channelInit(server->listener->fd, ReadEvent, acceptConnection, NULL,NULL,server);
		//(struct Channel*)malloc(sizeof(struct Channel));
	eventLoopAddTask(server->mainLoop, channel,ADD);
	eventLoopRun(server->mainLoop);




}

