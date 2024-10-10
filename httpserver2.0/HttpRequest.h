#pragma once
//����ͷ��ֵ��
#include "Buffer.h"
#include "HttpResponse.h"
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
struct RequestHeader {
	char* key;
	char* value;
};
//��ǰ�Ľ���״̬
enum HttpRequestState {
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,			//��������ݿ�
	ParseReqDone
};
//����Http����ṹ��
struct HttpRequest {
	char* method;
	char* url;
	char* version;
	struct RequestHeader* reqHeaders;
	int reqHeadersNum;				//��������Ч��Ԫ�ظ���
	enum HttpRequestState curState;
};
//��ʼ��
struct HttpRequest* httpRequestInit();
//����
void httpRequestReset(struct HttpRequest*req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestory(struct HttpRequest* req);
//��ȡ����״̬
enum HttpRequestState HttpRequestState(struct HttpRequest* request);
//�������ͷ
void httpRequestAddHeader(struct HttpRequest*request,const char*key,const char*value);
//����key�õ�����ͷ��value
const char* httpRequestGetHeader(struct HttpRequest* request, const char* key);
//����������
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);
//����������
char* bufferFindCRLF(struct Buffer*buffer);
//��������ͷ
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);
//����http����Э��
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,struct HttpResponse*response,struct Buffer*sendBuf,int socket );
//����http����Э��
bool processHttpRequest(struct HttpRequest* request,struct HttpResponse*response);
//���ַ�ת��Ϊ������
int HexToDec(char c);
//�����ַ���
void decodeMsg(char* to, char* from);
const char* getFileType(const char* name);
void sendDir(const char* filename, struct Buffer* sendBuf, int cfd);
void sendFile(const char* filename, struct Buffer* sendBuf, int cfd);