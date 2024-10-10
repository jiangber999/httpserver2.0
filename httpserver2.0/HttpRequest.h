#pragma once
//请求头键值对
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
//当前的解析状态
enum HttpRequestState {
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,			//请求的数据块
	ParseReqDone
};
//定义Http请求结构体
struct HttpRequest {
	char* method;
	char* url;
	char* version;
	struct RequestHeader* reqHeaders;
	int reqHeadersNum;				//数组内有效的元素个数
	enum HttpRequestState curState;
};
//初始化
struct HttpRequest* httpRequestInit();
//重置
void httpRequestReset(struct HttpRequest*req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestory(struct HttpRequest* req);
//获取处理状态
enum HttpRequestState HttpRequestState(struct HttpRequest* request);
//添加请求头
void httpRequestAddHeader(struct HttpRequest*request,const char*key,const char*value);
//根据key得到请求头的value
const char* httpRequestGetHeader(struct HttpRequest* request, const char* key);
//解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);
//读出请求行
char* bufferFindCRLF(struct Buffer*buffer);
//解析请求头
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);
//解析http请求协议
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,struct HttpResponse*response,struct Buffer*sendBuf,int socket );
//处理http请求协议
bool processHttpRequest(struct HttpRequest* request,struct HttpResponse*response);
//将字符转化为整形数
int HexToDec(char c);
//解码字符串
void decodeMsg(char* to, char* from);
const char* getFileType(const char* name);
void sendDir(const char* filename, struct Buffer* sendBuf, int cfd);
void sendFile(const char* filename, struct Buffer* sendBuf, int cfd);