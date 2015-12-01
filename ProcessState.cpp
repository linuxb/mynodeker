/*
 * ProcessState.cpp
 *
 *  Created on: 2015-11-21
 *      Author: linuxb
 */

#include "ProcessState.h"

ProcessState::ProcessState(uv_loop_t* pLoop,uv_async_t* itcBridge) {
	// TODO Auto-generated constructor stub
	this->mpLoop = pLoop;
	this->mpAsyncWatcher = itcBridge;
}

ProcessState::~ProcessState() {
	// TODO Auto-generated destructor stub
}

/*Handle the IO Task Async*/
void ProcessState::EventHandler()
{
	//msg
	void* msg = mp_evPump->plast;
	SpawnNewThread(msg);
}

void ProcessState::knode_ev_register(void(*iotask)(void*),void(*_cb_)(void*))
{
	if(mp_evPump->header == NULL)
		mp_evPump = createEvPump();
	if(!pushToEvPump(iotask,_cb_,mp_evPump))
		std::cerr<<"runtimeException:faild to push io event to threadpool\n"<<std::endl;
}

/*Initialize the Event Pump*/
void ProcessState::InitEventPump()
{
	if(this->mp_evPump == NULL)
	{
		mp_evPump = createEvPump();
	}
	//init the itc loop
	//send and recieve message between the worker and the main thread
	if(mpLoop == NULL)
		mpLoop = uv_default_loop();
	uv_async_init(mpLoop,mpAsyncWatcher,NotifyToV8);
}

void reFreshToMainThread(uv_async_t* async,uv_work_t* req)
{
	//test
	void* _progess_;
	void* node_addr_arr[2];
	node_addr_arr[0] = _progess_;
	node_addr_arr[1] = req;
	//
}

/*IO Task Handler Wrapper*/
void ProcessState::IOTaskWrapper(uv_work_t* req)
{
	//call
	TaskNode* callee = (TaskNode*)req->data;
	void* _argv = callee->argv;
	callee->IOWorkTask(_argv);
	reFreshToMainThread(mpAsyncWatcher,req);
	//notify to main thread
	uv_async_send(mpAsyncWatcher);
}

void ProcessState::RetreatThreadToPool(uv_work_t* req,int status)
{
	std::cout<<"IO Task finish\n"<<status<<std::endl;
	//free the listener
	uv_close((uv_handle_s*)req,NULL);

	/*fullfilled status*/
	*reinterpret_cast<int*>(req->reserved[0]) = KNODE_IO_TASK_FULLFILLED;
}

