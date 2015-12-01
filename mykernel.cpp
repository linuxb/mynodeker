/*
 * mykernel.cpp
 *
 *  Created on: 2015-11-21
 *      Author: linuxb
 */

#include "mykernel.h"

///工具方法

///构造looper队列
LooperQueue* NewLooper()
{
	return new LooperQueue;
}

///插入looper队列
bool enInvokeQueue(void(*wcb)(void*), LooperQueue* &queue)
{
	//检测空指针
	if (queue == NULL)
		queue = NewLooper();
	//空队列
	AscynInvokeQueueNode* pnode = new AscynInvokeQueueNode;
	if (pnode == NULL)
		throw std::bad_alloc();
	pnode->WrappedCallback = wcb;
	pnode->pNext = NULL;
	queue->QTail->pNext = pnode;
	queue->QTail = pnode;
	queue->qNum++;
	return true;
}

///从looper队列中删除
bool delectInvokeQueue(LooperQueue* &queue)
{
	//_nullptr_barr_(queue == NULL);
	//空队列异常
	if (queue->QHeader == NULL || queue->QHeader->pNext == NULL)
		return false;
	AscynInvokeQueueNode* rnode = queue->QHeader->pNext;
	queue->QHeader->pNext = rnode->pNext;
	if (queue->QTail == rnode)
		queue->QTail = queue->QHeader;
	delete rnode;
	queue->qNum--;
	return true;
}

AscynInvokeQueueNode* knode_get_first_item(LooperQueue* &queue)
{
	if(queue == NULL)
	{
		std::cerr<<"RuntimeException: reference of queue can not be NULL\n"<<std::endl;
		exit(1);
	}
	AscynInvokeQueueNode* node = queue->QHeader->pNext;
	queue->QHeader->pNext = node->pNext;
	queue->qNum--;
	return node;
}

///检测空队列
bool IsQueueEmpty(LooperQueue* &queue)
{
	return (queue->QHeader->pNext == NULL);
}

///Looper队列获取第一个任务
AscynInvokeQueueNode* getFirstInvoke(LooperQueue* &queue)
{
	if (IsQueueEmpty(queue))
		std::cout << "warning:no task in looper\n" << std::endl;
	return queue->QHeader->pNext;
}

///构造
EventHandlerPump* createEvPump()
{
	EventHandlerPump* ptr = new EventHandlerPump;
	if(ptr == NULL)
	{
		std::cout<<"nullPointerExcetion\n"<<std::endl;
		throw std::bad_alloc();
	}
	return ptr;
}

///添加到异步轮询列表
bool pushToEvPump(void(*cb)(void*), void(*iotask)(void*), EventHandlerPump* &pump)
{
	if (pump == NULL)
		pump = createEvPump();
	_log_out_("push an io task to the pump\n");
	//任务泵空
	Node* evnode = new Node;
	if (evnode == NULL)
		throw std::bad_alloc();
	evnode->callback = cb;
	evnode->IOWorkTask = iotask;
	pump->plast->next = evnode;
	evnode->pre = pump->plast;
	pump->plast = evnode;
	pump->plast->next = pump->header;
	pump->evNum++;
	return true;
}

///从轮询泵中删除
bool removeFromEvPump(Node* ev, EventHandlerPump* &pump)
{
	if (pump->header->next == NULL)
	{
		_log_out_("runtime exception:empty event pump\n");
		return false;
	}
	ev->pre->next = ev->next;
	if (ev->next != NULL)
		ev->next->pre = ev->pre;
	//环状态
	if (pump->plast == ev)
		pump->plast = ev->pre;
	delete ev;
	pump->evNum--;
	return true;
}

///轮询io事件泵
void evTravelPumpLoop(void(*watcher)(Node*), EventHandlerPump* &pump)
{
	if (pump == NULL)
		pump = createEvPump();
	Node* itor;
	if (pump->evNum == 0)
	{
		_log_out_("no io event in pump\n");
		return;
	}
	itor = pump->header->next;
	while (itor->next != pump->header)
	{
		Node* ptr = itor;
		itor = itor->next;

		/*apply*/
		watcher(ptr);
	}
	//尾节点
	watcher(itor);
}

///销毁事件泵
void destroyEvPump(const EventHandlerPump* pump)
{
	//未构建（构造函数已经创建）
	//若异常，构造函数执行失败
	if (pump->header == NULL)
		_assert_err_null("runtime exception:constructor excute failed\n");
	//无io事件
	if (pump->plast == pump->header)
	{
		free(pump->header);
		return;
	}
	Node* ptr = pump->header->next;
	Node* qnode;
	while (ptr->next != pump->header)
	{
		qnode = ptr;
		ptr = ptr->next;
		delete qnode;
	}
	//尾节点释放内存
	if (ptr != NULL)
		delete ptr;
	//头指针释放
	if (pump->header != NULL)
		free(pump->header);
}

///looper执行队列的销毁
void killAscynLooper(LooperQueue* queue)
{
	if (IsQueueEmpty(queue))
	{
		_log_out_("exception:empty looper\n");
#ifdef _DEBUG
		system("pause");
#endif
		if (queue->QHeader != NULL)
			free(queue->QHeader);
		return;
	}
	AscynInvokeQueueNode* pnode = queue->QHeader->pNext;
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
	if (queue->QHeader != NULL)
		free(queue->QHeader);
}




