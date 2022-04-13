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
	volatile bool mode_flag;
	std::atomic_uint32_t index_request;
	std::atomic_uint32_t active_threads;
	volatile uint32_t active_index;
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
		
		if (active_threads.fetch_add(1) == 0) {
			//indicate that the currently running thread started itself
			mode_flag = 1;

			printf("Thread is executing as a head\n");
			return;
		}

		//what if the DuplicateHandle process takes too long? The fetch add ^ will increment forcing the active thread to execute as a head but the handle buffer wont be filled.
		//it will call unlock on a freezed thread and deadlock the system.
		//we need to prevent resumeThread from being called unless the handle buffer for the next thread is filled.
		
		//this problem is more general than I thought and applies to all threads during the unlock phase. If the next thread hasn't completed it's setup it wont execute correctly.
		//easiest way to fix this is to maybe have a semaphore that indicates if the if the next thread has completed setup or not. 
		//how do we stop the primary thread from waiting on the next thread then?
		//I guess we dont but we only need wait until stupid windows functions end


		//if I am not the only thread that means there are other threads active, I must request a buffer index
		int index = reserve_index();
		//add our handle to the index
		DuplicateHandle(GetCurrentProcess(),
			GetCurrentThread(),
			GetCurrentProcess(),
			thread_handles + index,
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
				

				//*(thread_handles[active_index]) = 0;
				active_index = next_index(active_index);
				printf("Thread is a tail and incremented index to %d\n", active_index);
			}
			//we are a tail and a head, no need to activate next link
			else {
				printf("Thread is a lone link and activated nobody\n"); 
			}
		}
		//there are threads behind us in the chain
		else {
			//we are the chain head, we must activate the next link
			if (mode_flag == 1) {
				
				//*(thread_handles[active_index]) = 1;
				ResumeThread(thread_handles[active_index]);
				printf("Thread is a head and activated index %d\n", active_index);
			}
			//we are a chain link, we must deacitvate our go_flag, update the active_index, and activate its go_flag
			else {
				
				//*(thread_handles[active_index]) = 0;
				active_index = next_index(active_index);
				
				//printf("%d\n",active_index);
				ResumeThread(thread_handles[active_index]);
				printf("Thread is a link and activated + incremented index to index %d\n", active_index);
			}
		}
	}

} win32_mutex;
