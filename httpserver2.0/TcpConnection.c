#include "TcpConnection.h"
#include <stdlib.h>
int processRead(void *arg) {
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	int count=bufferSocketRead(conn->readBuf, conn->channel->fd);
	if (count > 0) {
		//接收到了http请求，解析http请求
		
#ifdef MSG_SEND_AUTO

		writeEventEnable(conn->channel, true);
		eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
#endif
		bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, conn->channel->fd);
		if (!flag) {
		//解析失败，回复一个简单的html
		char* errMsg = "http/1.1 400 Bad Request\r\n\r\n";
		bufferAppendString(conn->writeBuf, errMsg);
		//断开连接
		}

	}

 #ifndef MSG_SEND_AUTO

	//断开连接
	eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
#endif
	return 0;

}
int processwrite(void *arg) {
	struct TcpConnection *conn = (struct TcpConnection*)arg;
	//发送数据
	int count=bufferSendData(conn->writeBuf, conn->channel->fd);
	if (count > 0) {
		//判断数据是否被全部发送出去了
		if (bufferReadableSize(conn->writeBuf) == 0) {
			//1.不再检测写事件--修改Channel中的保存事件
			writeEventEnable(conn->channel, false);
			//2.修改dispatcher检测的集合--添加任务节点
			eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
			//3.删除这个节点
			eventLoopAddTask(conn->evLoop, conn->channel, DELETE);

		}
	}
	return 0;
}
struct TcpConnection* TcpConnectionInit(struct EventLoop*evLoop,int fd)
{
	struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
	conn->readBuf=bufferInit(10240);
	conn->writeBuf = bufferInit(10240);
	conn->evLoop = evLoop;
	conn->channel = channelInit(fd, ReadEvent, processRead, processwrite,tcpConnectionDestroy,conn);
	sprintf(conn->name, "connection-%d", fd);
	eventLoopAddTask(evLoop, conn->channel, ADD);
	conn->request = httpRequestInit();
	conn->response = httpResponseInit();
	return conn;
}

int tcpConnectionDestroy(void*arg)
{
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	if (conn != NULL) {
		if (conn->readBuf && bufferReadableSize(conn->readBuf)==0
			&& conn->writeBuf && bufferWriteableSize(conn->writeBuf)==0) {
			destroyChannel(conn->evLoop, conn->channel);
			httpRequestDestory(conn->request);
			httpResponseDestory(conn->response);
			bufferDestroy(conn->readBuf);
			bufferDestroy(conn->writeBuf);
			free(conn);
		}
		
	}
	return 0;
}
