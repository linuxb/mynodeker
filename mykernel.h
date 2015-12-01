#ifndef NODEKER_H
#define NODEKER_H

#pragma message("compiling the nodekernel...")

#include <uv.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <vector>
#include <list>

#ifdef linux
#include "pthread.h"
#endif

///跨平台开发

#define KNODE_IO_TASK_EV 0
#define KNODE_CALLBACK_EV 1
#define KNODE_IO_TASK_FULLFILLED 1
#define KNODE_IO_TASK_PENDING 0

#ifdef _WIN32
#include <Windows.h>
#elif linux
///
#endif

#ifdef __cplusplus
///扩展C部分
extern "C"{
	///
}
#endif

#define _log_out_(_msg_)	\
	std::cout << _msg_ << std::endl;

#ifdef _DEBUG
#define _assert_err_null(_x_)	\
	printf(_x_);	\
	system("pause");
#else
#define _assert_err_null(_x_)\
	std::cout << _x_ << std::endl;\
	exit(1);
#endif

#ifdef _WIN32
#define _nullptr_barr_(_ptrexp)	\
if (_ptrexp)		\
{				\
	std::cout << "null argument\n" << std::endl;					\
	throw std::invalid_argument("argument can not be null");	\
	MessageBox(0, (LPCWSTR)L"nullpoiterException", (LPCWSTR)L"Error", MB_OK);\
	exit(1);	\
}
//unix系统兼容
#else linux
#define _nullptr_barr_(_ptrexp)	\
if(_ptrexp)	\
{	\
	assert_err_null("argument can not be null");	\
}
#endif


///v8引擎封装提供js接口
///node内核执行队列
typedef struct LinkNode
{
	//回调项
	void(*WrappedCallback)(void*);
	void* disc;
	void* argv;
	void(*_cb_)(void*);
	struct LinkNode* pNext;
} AscynInvokeQueueNode;

typedef struct Queue
{
	/*互斥锁*/
#ifdef linux
	pthread_mutex_t m_kn_mutex;
#endif
	AscynInvokeQueueNode* QHeader;
	AscynInvokeQueueNode* QTail;
	int qNum;

#ifdef __cplusplus
	Queue()
	{
		//AscynInvokeQueueNode* ptr = new AscynInvokeQueueNode;
		AscynInvokeQueueNode* ptr = (AscynInvokeQueueNode*)malloc(sizeof(AscynInvokeQueueNode));
		//程序最后组要调用free函数防止内存泄露
		if (ptr == NULL)
			throw std::bad_alloc();
		ptr->pNext = NULL;
		this->qNum = 0;
		this->QHeader = ptr;
		this->QTail = this->QHeader;
	}
	//析构
	~Queue()
	{
		if (this->QHeader == this->QTail)
		{
			_log_out_("exception:empty looper\n");
#ifdef _DEBUG
			system("pause");
#endif
			if (this->QHeader != NULL)
				free(this->QHeader);
			return;
		}
		AscynInvokeQueueNode* pnode = this->QHeader->pNext;
		AscynInvokeQueueNode* qnode;
		while (pnode->pNext != NULL)
		{
			qnode = pnode;
			pnode = pnode->pNext;
			delete qnode;
		}
		//尾节点释放
		if (pnode != NULL)
			delete pnode;
		//头节点释放
		if (this->QHeader != NULL)
			free(this->QHeader);;
	}

	bool isEmpty()
	{
		return (this->qNum == 0)?true:false;
	}

	AscynInvokeQueueNode*
#endif
} LooperQueue;

///内核异步IO事件链表
///使用libuv线程池统一调用
///双向链表
typedef struct Node
{
	void(*IOWorkTask)(void*);
	void(*callback)(void*);
	uv_work_t req;
	//IO参数列表地址
	void* argv;
	//参数个数
	int argc;
	//promise notify method
	void(*_notify_cb_)(void*);
	struct Node* pre;
	struct Node* next;
} TaskNode;

typedef struct LinkList
{
	TaskNode* header;
	TaskNode* plast;
	int evNum;
	void* disc;
#ifdef __cplusplus
	LinkList()
	{
		Node* ptr = (Node*)malloc(sizeof(Node));
		if (ptr == NULL)
			throw std::bad_alloc();
		ptr->next = NULL;
		ptr->pre = NULL;
		this->evNum = 0;
		this->header = ptr;
		this->plast = this->header;
	}
	//析构
	~LinkList()
	{
		if (this->header == NULL)
			_assert_err_null("runtime exception:constructor excute failed\n");
		//无io事件
		if (this->plast == this->header)
		{
			_log_out_("empty event pump\n");
			free(this->header);
		}
		else
		{
			Node* ptr = this->header->next;
			Node* qnode;
			while (ptr->next != this->header)
			{
				qnode = ptr;
				ptr = ptr->next;
				delete qnode;
			}
			//尾节点释放内存
			if (ptr != NULL)
				delete ptr;
			//头指针释放
			if (this->header != NULL)
				free(this->header);
		}
	}
#endif
} EventHandlerPump;

EventHandlerPump* createEvPump();
void destroyEvPump(EventHandlerPump*);
LooperQueue* NewLooper();
void killAscynLooper(LooperQueue*);


#endif
