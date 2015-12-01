/*
 * nodeInvokeLooper.cpp
 *
 *  Created on: 2015-11-21
 *      Author: linuxb
 */

#include "nodeInvokeLooper.h"

NodeInvokeLooper::NodeInvokeLooper(uv_loop_t* mLoop) : mpLoop(mLoop)
{
	mpLooper = create_invoke_looper_queue();
}

NodeInvokeLooper::~NodeInvokeLooper()
{
	/*destroy lock*/
	pthread_mutex_destroy(&mpLooper->m_kn_mutex);
}

LooperQueue* create_invoke_looper_queue()
{
	return new LooperQueue;
}

void NodeInvokeLooper::InitLooper(uv_async_t mAsync)
{
	if(mpLoop == NULL)
		mpLoop = uv_default_loop();
	if(mpState == NULL)
		mpState = ProcessState::myProcessState(mpLoop,&mAsync);
	if(mpLooper->QHeader == NULL)
		mpLooper = create_invoke_looper_queue();
	/*initialize thread lock*/
	pthread_mutex_init(&mpLooper->m_kn_mutex,NULL);
}


kn_status NodeInvokeLooper::observe_ev_pump()
{
	/*Travel event pump*/
	evTravelPumpLoop(NodeInvokeLooper::kn_evnode_handler,mpState->mp_evPump);
}

kn_status NodeInvokeLooper::watcher_for_v8_op()
{
	/*start watcher thread*/
	if(pthread_create(&m_watcher_thread,NULL,NodeInvokeLooper::v8_process_listener,NULL) == -1)
		std::cerr<<"runtimeException:watcher thread launch failed\n"<<std::endl;
}

void NodeInvokeLooper::runLooper()
{
	while(true)
	{
		while(!mpLooper->isEmpty())
		{
			/*lock for main thread*/
			int ret = pthread_mutex_lock(&mpLooper->m_kn_mutex);
			if(EBUSY == ret)
				std::cout<<"ThreadLockException:main looper busy\n"<<std::endl;
			else if(ret != 0)
				perror("Error:can not obtain the mutex of main looper\n");
			AscynInvokeQueueNode* task = knode_get_first_item(mpLooper);
			if(*reinterpret_cast<int*>(task->disc) == KNODE_IO_TASK_EV)
			{
				//IO Task
				mpState->knode_ev_register(task->WrappedCallback,task->_cb_);
				mpState->EventHandler();
			}
			if(*reinterpret_cast<int*>(task->disc) == KNODE_CALLBACK_EV)
			{
				//callback of fullfilled io task
				/*apply*/
				task->WrappedCallback(task->argv);
			}
			/*free memory*/
			if(task != NULL)
				delete task;
			/*release lock*/
			if(pthread_mutex_unlock(&mpLooper->m_kn_mutex) != 0 )
				std::cerr<<"runtimeException:can not release the lock(watcher thread)\n"<<std::endl;
		}
		observe_ev_pump();
	}
}
