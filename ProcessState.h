/*
 * ProcessState.h
 *
 *  Created on: 2015-11-21
 *      Author: linuxb
 */

#ifndef PROCESSSTATE_H_
#define PROCESSSTATE_H_

#include  "mykernel.h"


class ProcessState {
public:
	virtual ~ProcessState();

	void InitEventPump();
	//异步事件泵
	EventHandlerPump* mp_evPump;
	//分配工作线程
	void EventHandler();
	void NotifyMainLooper(void* &);
    void RetreatThreadToPool(uv_work_t*,int);
    void IOTaskWrapper(uv_work_t*);

    void knode_ev_register(void(*)(void*),void(*)(void*));

    static ProcessState* myProcessState(uv_loop_t* mLoop,uv_async_t* mAsync)
	{
    	if(ProcessState::mpState == NULL)
    		ProcessState::mpState =  new ProcessState(mLoop,mAsync);
    	return ProcessState::mpState;
	}
protected:
    inline void SpawnNewThread(void*msg = NULL)
    {
    	//listener
    	uv_work_t req = mp_evPump->plast->req;
    	req.data = msg;
    	*reinterpret_cast<int*>(req.reserved[0]) = KNODE_IO_TASK_PENDING;
    	uv_queue_work(mpLoop,&req,ProcessState::IOTaskWrapper,ProcessState::RetreatThreadToPool);
    }

    //ipc with v8 process
    inline void NotifyToV8(uv_async_t* observer)
    {
    	//to do
    	//socket
    	void* arg = (void*[2])observer->data;
    	std::cout<<arg[0]<<std::endl;
    	void(*notify_callee)(void*) = ((TaskNode*)((uv_work_t*)arg[1])->data)->_notify_cb_;
    	//call notify
    	notify_callee(arg[0]);
    }
private:
	ProcessState(uv_loop_t*,uv_async_t*);
	//事件主循环引用
	uv_loop_t* mpLoop;
	uv_async_t* mpAsyncWatcher;
	static ProcessState* mpState;

	class GCWorker
	{
	public:
		~GCWorker()
		{
			if(ProcessState::mpState != NULL)
			{
				delete ProcessState::mpState;
				std::cout<<"ThreadPool destroyed...\n"<<std::endl;
			}
		}
	}

	//析构

	GCWorker gc_worker;
};

#endif /* PROCESSSTATE_H_ */
