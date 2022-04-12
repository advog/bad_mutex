#pragma once
#include <atomic>
#include <thread>
#include <iostream>

//use fetch_add


typedef struct atomic_mutex_struct{
	int num_threads;
	bool mode_flag;
	std::atomic_uint32_t index_request;
	std::atomic_uint32_t active_threads;
	uint32_t active_index;
	volatile bool** go_flags;

	void initialize(int n) {
		num_threads = n;
		go_flags = (volatile bool**)malloc(sizeof(bool*) * num_threads);
		for (int i = 0; i < num_threads; i++) {
			go_flags[i] = (bool*)calloc(1, sizeof(bool));
		}
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

	void lock(int thread_id) {
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
		

		//this emulates a hardware local spinner
		volatile bool local_spinner = 0;
		//adds a pointer that points from the buffer to the "local spinner"
		go_flags[index] = &local_spinner;
		
		//spin on local spinner
		while (local_spinner == 0){ std::this_thread::sleep_for(std::chrono::nanoseconds(1000));}
		//printf("Thread %d is executing as link in index %d\n", thread_id, index);
		//once we pass local spinner we are in 
		mode_flag = 0;
	}

	void unlock(int thread_id) {
		//printf("Thread %d is beginning unlock process    ", thread_id);
		//printf("MODE FLAG %d    ", mode_flag);
		//we are a tail 
		if (active_threads.fetch_add(-1) == 1) {
			//we are the last thread of a chain, we need to update active_index and deactivate our go_flag
			if (mode_flag == 0) {
				//printf("Thread %d is a tail and activated index %d\n", thread_id, active_index);
				*(go_flags[active_index]) = 0;
				active_index++;
				
				
			}
			//we are a tail and a head, no need to activate next link
			else { 
				//printf("Thread %d is a lone thread and activated nobody\n", thread_id); 
			}
		}
		//there are threads behind us in the chain
		else {
			//we are the chain head, we must activate the next link
			if (mode_flag == 1) {
				//printf("Thread %d is a head and activated index %d\n", thread_id, active_index);
				*(go_flags[active_index]) = 1;
				
			}
			//we are a chain link, we must deacitvate our go_flag, update the active_index, and activate its go_flag
			else {
				//printf("Thread %d is a link and activated index %d\n", thread_id, active_index);
				*(go_flags[active_index]) = 0;
				active_index = next_index(active_index);
				*(go_flags[active_index]) = 1;
				
			}
		}
	}

} atomic_mutex;


