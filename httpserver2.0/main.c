#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "TcpServer.h"
#include <arpa/inet.h>
#include <string.h>
#include "Log.h"
int main(int argc,char*argv[]) {
	//if (argc < 3) {
	//	printf("./a.out port path\n");
	//	return -1;
	//}
	//unsigned short port = atoi(argv[1]);
	unsigned short port = atoi("10000");
	//�л�����Ŀ¼
	chdir("/home/dog/resource");
	//����������
	struct TcpSerer* server = tcpserverInit(port, 4);
	tcpServerRun(server);
	return 0;
}