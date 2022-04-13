#pragma once


#include <atomic>
#include <thread>
#include <iostream>
#include "windows.h"

#pragma optimize( "", off )
//use sleap and resumeThread functions\
//instead if local spinners threads write their handle to the bool**

typedef struct win32_mutex_struct {
	uint32_t num_threads;
	bool mode_flag;
	std::atomic_uint32_t index_request;
	std::atomic_uint32_t active_threads;
	uint32_t active_index;
	HANDLE* thread_handles;

	void initialize(int n) {
		num_threads = n;
		thread_handles = (HANDLE*)malloc(sizeof(HANDLE) * num_threads);
		index_request = 0;
		active_threads = 0;
		active_index = 0;
		mode_flag = 0;
	}

	int reserve_index() {
		return index_request.fetch_add(1) % num_threads;
	}

	int next_index(int n) {
		return (n + 1) % num_threads;
	}

	void check_if_alive() {
		HANDLE this_thread = GetCurrentThread();
		//printf("%d", this_thread);
	}

	void lock() {
		//add 1 to active threads
		//if I am the only thread then start
		if (active_threads.fetch_add(1) == 1) {
			//indicate that the currently running thread started itself
			mode_flag = 1;

			//printf("Thread %d is executing as a head\n", thread_id);
			return;
		}

		//if I am not the only thread that means there are other threads active, I must request a buffer index
		int index = reserve_index();

		//add our handle to the index
		DuplicateHandle(GetCurrentProcess(),
			GetCurrentThread(),
			GetCurrentProcess(),
			thread_handles+index,
			0,
			TRUE,
			DUPLICATE_SAME_ACCESS);

		//spin on local spinner
		SuspendThread(thread_handles[index]);
		//printf("Thread %d is executing as link in index %d\n", thread_id, index);
		//once we pass local spinner we are in 
		mode_flag = 0;
	}

	void unlock() {
		//printf("Thread %d is beginning unlock process    ", thread_id);
		//printf("MODE FLAG %d    ", mode_flag);
		//we are a tail 
		if (active_threads.fetch_add(-1) == 1) {
			//we are the last thread of a chain, we need to update active_index and deactivate our go_flag
			if (mode_flag == 0) {
				//printf("Thread is a tail and activated index %d\n", active_index);

				//*(thread_handles[active_index]) = 0;
				active_index = next_index(active_index);
			}
			//we are a tail and a head, no need to activate next link
			else {
				//printf("Thread is a lone thread and activated nobody\n"); 
			}
		}
		//there are threads behind us in the chain
		else {
			//we are the chain head, we must activate the next link
			if (mode_flag == 1) {
				//printf("Thread is a head and activated index %d\n", active_index);
				//*(thread_handles[active_index]) = 1;
				ResumeThread(thread_handles[active_index]);
			}
			//we are a chain link, we must deacitvate our go_flag, update the active_index, and activate its go_flag
			else {
				//printf("Thread is a link and activated index %d\n", active_index);
				//*(thread_handles[active_index]) = 0;
				active_index = next_index(active_index);
				//printf("%d\n",active_index);
				ResumeThread(thread_handles[active_index]);
			}
		}
	}

} win32_mutex;
