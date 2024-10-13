#define _GNU_SOURCE
#include "HttpRequest.h"
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "TcpConnection.h"
#include <ctype.h>
#include "Log.h"
#define HeaderSize 12
struct HttpRequest* httpRequestInit()
{
	struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
	httpRequestReset(request);
	request->reqHeaders = (struct RequestHeader*)malloc(sizeof(struct RequestHeader)*HeaderSize);
	return request;
}

void httpRequestReset(struct HttpRequest* req)
{
	req->curState = ParseReqLine;
	req->method = NULL;
	req->url = NULL;
	req->version = NULL;
	req->reqHeadersNum = 0;
}
void httpRequestResetEx(struct HttpRequest* req)
{
	free(req->method);
	free(req->url);
	free(req->version);
	if (req->reqHeaders != NULL) {
		int i = 0;
		for (i = 0; i < req->reqHeadersNum; i++) {
			free(req->reqHeaders[i].key);
			free(req->reqHeaders[i].value);
		}
		free(req->reqHeaders);
	}
	httpRequestReset(req);
}
void httpRequestDestory(struct HttpRequest* req) {
	if (req != NULL) {
		httpRequestResetEx(req);
		free(req);
	}

}

enum HttpRequestState HttpRequestState(struct HttpRequest* request)
{
	return request->curState;
}

void httpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value)
{
	request->reqHeaders[request->reqHeadersNum].key = (char*)key;
	request->reqHeaders[request->reqHeadersNum].value = (char*)value;
	request->reqHeadersNum++;
}
const char* httpRequestGetHeader(struct HttpRequest* request, const char* key) {
	if (request != NULL) {
		int i = 0;
		for (i=0; i < request->reqHeadersNum; i++) {
			if (!(strncasecmp(request->reqHeaders[i].key, key, strlen(key)))){
				return request->reqHeaders[i].value;
			}

		}
	}
	
	return NULL;

}
char*  splitRequestLine(const char*start,const char*end,const char*sub,char**ptr) {
	char* space=(char*)end;
	if (sub != NULL) {
		space = memmem(start, end-start, sub, strlen(sub));
		assert(space!=NULL);
	}
	int Lenth = space - start;
	char*tmp = (char*)malloc(Lenth + 1);
	strncpy(tmp, start, Lenth);
	tmp[Lenth] = '\0';
	*ptr = tmp;
	return space + 1;
}
bool parseHttpRequestLine(struct HttpRequest* request,struct Buffer* readBuf)
{
	//读出请求行，保存字符串结束地址
	char* end = bufferFindCRLF(readBuf);
	//保存字符串起始地址
	char* start = readBuf->data + readBuf->readPos;
	//请求行长度
	int lineSize = end - start;
	if (lineSize) {
		start = splitRequestLine(start, end, " ", &request->method);
		start = splitRequestLine(start, end, " ", &request->url);
		splitRequestLine(start, end, NULL, &request->version);

		////get /xx/xx.txt http/1.1
		////请求方式
		//char* space = memmem(start, lineSize," ",1);
		//assert(space);
		//int methodSize = space - start;
		//request->method=(char*)malloc(methodSize+1);
		//strncpy(request->method, start, methodSize);
		//request->method[methodSize] = '\0';

		////请求的静态资源
		//start =space+1;
		//space = memmem(start,end-start," ",1);
		//assert(space);
		//int urlSize = space - start;
		//request->url = (char*)malloc(urlSize + 1);
		//strncpy(request->url, start, urlSize);
		//request->url[urlSize] = '\0';
		////请求的http版本
		//start =space+ 1;
		//int versionSize = end - start;
		//request->version = (char*)malloc(versionSize + 1);
		//strncpy(request->version,start,versionSize);
		//request->version[versionSize] = '\0';

		//为解析请求头做准备
		readBuf->readPos += lineSize;
		readBuf->readPos += 2;
		//修改状态
		request->curState = ParseReqHeaders;
		return true;
	}
	return false;
}


//该函数处理请求头中的一行
bool parseHttpRequestHeader(struct HttpRequest* request,struct Buffer* readBuf)
{	char* end = bufferFindCRLF(readBuf);
	if (end != NULL) {
		char* start = readBuf->data+readBuf->readPos;
		int lineSize = end - start;
		//基于：搜索字符串
		char*middle=memmem(start,lineSize, ": ", 2);
		if (middle != NULL) {
			char* key = (char*)malloc(middle - start+1);
			strncpy(key, start, middle-start);
			key[middle - start] = '\0';
			char* value = (char*)malloc(end - middle -2+1);//注意是大小
			strncpy(value, middle + 2, end - middle - 2);
			value[middle - start-2] = '\0';
			httpRequestAddHeader(request, key, value);
			//移动读数据位置
			readBuf->readPos += lineSize;
			readBuf->readPos += 2;
		}
		else {
			//请求头被解析完了，跳过空行
			readBuf->readPos += 2;
			//修改解析状态
			//忽略post请求，按get请求处理
			request->curState = ParseReqDone;
		}
		return true;
	}
	return false;
}

bool parseHttpRequest(struct HttpRequest* request,struct  Buffer* readBuf,
	struct HttpResponse* response, struct Buffer* sendBuf, int socket)
{
	bool flag = true;
	while (request->curState!=ParseReqDone) {
		switch (request->curState) {
			case ParseReqLine:
				flag=parseHttpRequestLine(request, readBuf);
				break;
			case ParseReqHeaders:
				flag=parseHttpRequestHeader(request, readBuf);
				break;
			case ParseReqBody:
				break;
			default:
				break;
		}
		if (!flag) {
			return false;
		}
		//判断是否解析完毕了，如果完毕了，需要准备回复的数据
		if (request->curState == ParseReqDone) {
			//1.根据解析出的原始数据，对客户单的请求做出处理
			processHttpRequest(request,response);
			//2.组织响应数据并发送给客户端
			httpResponsePrepareMsg(response, sendBuf, socket);
		}
	}
	request->curState = ParseReqLine;//还原状态，保证还能继续处理第二条及以后的请求
	return flag;
}
//处理基于get的http请求
bool processHttpRequest(struct HttpRequest* request,struct HttpResponse*response)
{
	if (strcasecmp(request->method, "get") != 0) {

		return -1;
	}
	//printf("yuan lai path = % s\n", path);
	decodeMsg(request->url, request->url);
	//printf("path = % s\n", path);
	//printf("method is %s,path is %s\n", method, path);
	//处理客户端请求的静态资源文件 文件或目录
	char file[32] = {0};
	if (strcmp(request->url, "/") == 0) {
		strcpy(file ,"./");
	}
	else {
		strcpy(file ,request->url + 1);
	}
	//获取文件属性
	struct stat st;
	int ret = stat(file, &st);
	if (ret == -1) {
		//发送一个404错误页面
		//sendHeadMsg(cfd, 404, "Not found", getFileType(".html"), -1);
		//sendFile(cfd, "404.html");
		strcpy(response->fileName, "404.html");
		strcpy(response->statusMsg, "Not Found");
		response->statusCode = NotFound;
		//响应头
		httpResponseAddHeader(response, "Content-type", getFileType(".html"));
		response->sendDataFunc=sendFile;

		return 0;
	}
	strcpy(response->fileName, file);
	strcpy(response->statusMsg, "OK");
	response->statusCode = OK;
	if (S_ISDIR(st.st_mode)) {
		//将目录的内容发送给客户端
		//sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
		//sendDir(file, cfd);
		//响应头
		httpResponseAddHeader(response, "Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;
	}
	else {
		//将文件的内容发送给客户端
		//sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
		//sendFile(cfd, file);
		//响应头
		char tmp[12] = {0};
		sprintf(tmp,"%ld", st.st_size);
		httpResponseAddHeader(response, "Content-type", getFileType(file));
		httpResponseAddHeader(response, "Content-length", tmp);
		response->sendDataFunc = sendFile;

	}
	return false;
}
//将字符转化为整形数
int HexToDec(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return 0;
}
void decodeMsg(char* to, char* from) {
	for (; *from != '\0'; from++, to++) {
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
			*to = HexToDec(from[1]) * 16 + HexToDec(from[2]);

			from += 2;
		}
		else {
			*to = *from;
		}
	}
	*to = '\0';

}
void sendFile(const char* filename, struct Buffer* sendBuf, int cfd)
{

	int fd = open(filename, O_RDONLY);
	printf("filename is %s\n", filename);
	assert(fd > 0);


#if 1
	while (1) {
		char buf[4096] = {};
		int len = read(fd, buf, sizeof buf);
		if (len > 0) {
			//send(cfd, buf, len, MSG_NOSIGNAL);
			bufferAppendData(sendBuf, buf,len);
#ifndef MSG_SEND_AUTO
			bufferSendData(sendBuf, cfd);
#endif 
			//usleep(10);				//防止客户端处理不过来数据
		}
		else if (len == 0) {
			break;
		}
		else {
			close(fd);
			perror("read");
			break;
		}
	}
#else
	off_t offset = 0;
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	while (offset < size) {
		usleep(10);
		int ret = sendfile(cfd, fd, &offset, size - offset);
		printf("ret value:%d\n", ret);
		if (ret == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				printf("no data");
			}
			else {
				perror("sendfile");
			}
		}

	}
#endif
	close(fd);

	return;
}
void sendDir(const char* filename, struct Buffer* sendBuf, int cfd)
{
	//遍历目录
	struct dirent** namelist;
	int num = scandir(filename, &namelist, NULL, alphasort);
	char buf[4096] = { 0 };

	sprintf(buf, "<html><head><title>%s</title></head><body><table>", filename);

	//取出文件名
	int i;
	for (i = 0; i < num; i++) {
		char* name = namelist[i]->d_name;
		
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", filename, name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode)) {
			//如果是目录
			//a标签 <a href="">name</a>
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);


		}
		else {
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
		bufferSendData(sendBuf, cfd);
#endif 
		//send(cfd, buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		free(namelist[i]);		//释放单个的内存
	}
	sprintf(buf, "</table></body></html>");
	bufferAppendString(sendBuf, buf);
#ifndef MSG_SEND_AUTO
	bufferSendData(sendBuf, cfd);
#endif 
	//send(cfd, buf, strlen(buf), 0);
	free(namelist);				//释放总体的内存
	return ;
}
const char* getFileType(const char* name)
{
	const char* dot = strrchr(name, '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8";
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";//纯文本
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".bmp") == 0)
		return "application/x-bmp";
	if (strcmp(dot, ".xml") == 0)
		return "text/xml";
	if (strcmp(dot, ".jpe") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mp3";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpg";
	if (strcmp(dot, ".midi") == 0)
		return "audio/mid";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".avi") == 0)
		return "video/avi";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";
	return "text/plain; charset=utf-8";
}