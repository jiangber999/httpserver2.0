#pragma once
#include "Buffer.h"
enum HttpStatusCode{
	Unknown,
	OK=200,			//����ɹ�
	MovePermanently=301,			//
	MovedTemporarily=302,
	BadRequest=400,
	NotFound=404,
};
//������Ӧ�Ľṹ��
struct ResponseHeader {
	char key[32];
	char value[128];
};
//����һ������ָ�룬������֯Ҫ�ظ����ͻ��˵����ݿ�
typedef	void(*responseBody)(const char* filename, struct Buffer* sendBuf, int socket);
//����ṹ��
struct HttpResponse {
	//״̬�У�״̬�룬״̬����
	enum HttpStatusCode statusCode;
	char statusMsg[128];
	//��Ӧͷ-��ֵ��
	struct ResponseHeader* headers;
	int headerNum;
	responseBody sendDataFunc;
	char fileName[128];

};
//��ʼ��
struct HttpResponse* httpResponseInit();
//����
void httpResponseDestory(struct HttpResponse* response);
//�����Ӧͷ
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value);
//��֯http����Ӧ����
void httpResponsePrepareMsg(struct HttpResponse* response, struct Buffer* sendBuf, int socket);


