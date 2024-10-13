#include "Buffer.h"
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include "Log.h"
struct Buffer* bufferInit(int size) {
	struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
	if (buffer != NULL) {
		buffer->readPos = buffer->writePos = 0;
		buffer->capacity = size;

		buffer->data = (char*)malloc(sizeof(char) * size);
		memset(buffer->data, 0, size);
	}

	return buffer;
}
void bufferDestroy(struct Buffer* buffer) {
	if (buffer != NULL) {
		if (buffer->data != NULL) {
			free(buffer->data);
		}
		
	}
	free(buffer);
}
void bufferExtendRoom(struct Buffer* buffer, int size) {
	//1.�ڴ湻�ò���Ҫ����
	if (bufferWriteableSize(buffer) >= size) {
		return;

	}
	//2.�ڴ���Ҫ�ϲ��Ź���-����Ҫ����
	//ʣ���д���ڴ�+�Ѷ����ڴ�>size
	else if (buffer->readPos+ bufferWriteableSize(buffer) >= size) {
		//�õ�δ�����ڴ��С
		int readable = bufferReadableSize(buffer);
		//�ƶ��ڴ�
		memcpy(buffer->data, buffer->data+buffer->readPos, readable);
		//����λ��
		buffer->readPos = 0;
		buffer->writePos = readable;
	}
	//3.�ڴ治����-����
	else {
		char* temp = (char*)realloc(buffer->data, size + buffer->capacity);
		if (temp == NULL) {
			return;				//ʧ����
		}
		//��չ�ڴ�
		memset(temp+buffer->capacity, 0, size);
		buffer->data = temp;
		buffer->capacity = buffer->capacity + size;
	}
}
int bufferWriteableSize(struct Buffer* buffer) {

	return buffer->capacity - buffer->writePos;
}
int bufferReadableSize(struct Buffer* buffer) {
	return buffer->writePos - buffer->readPos;
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
	if (buffer == NULL || data == NULL || size <= 0) {
		return -1;
	}
	//����
	bufferExtendRoom(buffer, size);
	//����
	memcpy(buffer->data+buffer->writePos,data,size);
	buffer->writePos += size;
	return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data)
{
	int size = strlen(data);
	int ret=bufferAppendData(buffer, data, size);
	return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
	//read/recv/readv
	int writeable = bufferWriteableSize(buffer);
	struct iovec vec[2];
	char* tmpbuf = (char*)malloc(40960);
	//��ʼ������Ԫ��
	vec[0].iov_base = buffer->data+buffer->writePos;
	vec[0].iov_len = writeable;
	vec[1].iov_base = tmpbuf;
	vec[1].iov_len = 40960;
	int result= readv(fd, vec, 2);
	if (result ==-1) {
		perror("readv");
		return -1;
	}
	else if(result<=writeable){
		buffer->writePos += result;
	}
	else {
		buffer->writePos = buffer->capacity;
		bufferAppendData(buffer, tmpbuf, result - writeable);
	} 
	free(tmpbuf);

	return result; 
}

int bufferSendData(struct Buffer* buffer, int socket)
{
	//�ж���������
	int readable = bufferReadableSize(buffer);
	if (readable>0) {
		int count = send(socket, buffer->data + buffer->readPos,readable ,MSG_NOSIGNAL);
		if (count) {
			buffer->readPos += count;
			//usleep(1);
		}
		return count;
	}

	return 0;
}
char* bufferFindCRLF(struct Buffer* buffer)
{
	char* ptr = memmem(buffer->data + buffer->readPos, bufferReadableSize(buffer), "\r\n", 2);
	return ptr;
}