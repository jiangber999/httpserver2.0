#include "HttpResponse.h"
#include <stdlib.h>
#include <string.h>
#define ResHeaderSize 16
struct HttpResponse* httpResponseInit()
{
	struct HttpResponse* response = (struct HttpResponse*)malloc(sizeof(struct HttpResponse));
	response->headerNum = 0;
	response->statusCode = Unknown;
	bzero(response->statusMsg, sizeof(response->statusMsg));
	
	int size = ResHeaderSize * sizeof( struct ResponseHeader);
	response->headers = (struct ResponseHeader*)malloc(size);
	//初始化数组
	bzero(response->headers, size);
	bzero(response->fileName, sizeof(response->fileName));
	//函数指针
	response->sendDataFunc = NULL;
	return response;
}

void httpResponseDestory(struct HttpResponse* response)
{
	if (response != NULL) {
		free(response->headers);
		free(response);
	}
}

void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value)
{
	if (response == NULL || key == NULL || value == NULL) {
		return;
	}
	strcpy(response->headers[response->headerNum].key, key);
	strcpy(response->headers[response->headerNum].value, value);
	response->headerNum++;
}

void httpResponsePrepareMsg(struct HttpResponse* response,struct  Buffer* sendBuf, int socket)
{
	//状态行
	char temp[1024];
	sprintf(temp, "http/1.1 %d %s\r\n", response->statusCode, response->statusMsg);
	bufferAppendString(sendBuf, temp);

	//响应头
	int i = 0;
	for (i = 0; i < response->headerNum; i++) {
		sprintf(temp, "%s: %s", response->headers[i].key, response->headers[i].value);
		bufferAppendString(sendBuf, temp);
		bzero(temp, sizeof(temp));
	}
	//空行
	bufferAppendString(sendBuf, "\r\n");
#ifndef MSG_SEND_AUTO
	bufferSendData(sendBuf,socket);
#endif 

	
	//回复的数据
	response->sendDataFunc(response->fileName, sendBuf, socket);
}
