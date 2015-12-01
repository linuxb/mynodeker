/*
 * nodeInvokeLooper.h
 *
 *  Created on: 2015-11-21
 *      Author: linuxb
 */

#ifndef NODEINVOKELOOPER_H_
#define NODEINVOKELOOPER_H_

#include "mykernel.h"
#include "ProcessState.h"

typedef bool kn_status;

struct v8_req_s
{
	void(*IOTask)(void*);
	void(*_cb_)(void*);
	void* argv;
}


///looper管理类
class NodeInvokeLooper
{
public:
	NodeInvokeLooper(uv_loop_t*);
	~NodeInvokeLooper();

	void InitLooper(uv_async_t);

	void runLooper();

	kn_status observe_ev_pump();

	//接受V8的指令
	kn_status watcher_for_v8_op();

protected:
	inline void kn_evnode_handler(Node* task)
	{
		if(*reinterpret_cast<int*>(task->req.reserved[0]) == KNODE_IO_TASK_FULLFILLED)
		{
			/*io task fullfilled*/
			/*add callback item to looper*/
			enInvokeQueue(task->callback,mpLooper);
			if(task != NULL)
				removeFromEvPump(task,mpState->mp_evPump);
		}
	}

	inline void* v8_process_listener(void* argv)
	{
		kn_status interface_status = false;
		/*worker thread*/
		/*itc with v8*/
		if(interface_status)
		{
			v8_req_s req;
			/*add v8 task item to looper queue*/
			/*线程安全*/
			/*try to obtain the mutex belong to main thread*/
			int ret = pthread_mutex_trylock(&mpLooper->m_kn_mutex);
			if(EBUSY == ret)
				std::cout<<"ThreadLockException:main looper busy\n"<<std::endl;
			else if(ret != 0)
				perror("Error:can not obtain the mutex of main looper\n");
			enInvokeQueue(req._cb_,mpLooper);
			/*release lock for main looper*/
			/*catch more task from v8 process*/
			if(pthread_mutex_unlock(&mpLooper->m_kn_mutex) != 0 )
				std::cerr<<"runtimeException:can not release the lock(watcher thread)\n"<<std::endl;
		}
		/*abort the communication with v8*/
		if(b_shutdowm_itc_v8)
			pthread_exit(NULL);
		return NULL;
	}

private:
	LooperQueue* mpLooper;
	uv_async_t mitcWacher;
	uv_loop_t* mpLoop;

	kn_status b_shutdowm_itc_v8 = false;

#ifdef linux
	pthread_t m_watcher_thread;
#endif
	ProcessState* mpState = NULL;
};

#endif /* NODEINVOKELOOPER_H_ */
