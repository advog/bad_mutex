#pragma once
#include <thread>

typedef struct heap_mutex_struct {
	bool* interest_heap;
	bool* victim_heap;
	int interest_heap_size;
	int victim_heap_size;

	int parent_of(int pos) {
		return (pos - 1) / 2;
	}

	//identical to parent of, in here for readability
	int victim_of(int pos) {
		return (pos - 1) / 2;
	}

	bool is_left(int pos) {
		return pos % 2;
	}

	int sibling_of(int pos) {
		if (is_left(pos)) { return pos + 1; }
		else { return pos - 1; }
	}

	void lock(int thread_id) {
		//thread always enters in same position (can update this later with new initial position function)
		int pos = thread_id + victim_heap_size;
		//indicate thread entry into heap
		interest_heap[pos] = 1;
		//iterate up heap until at root
		while (pos != 0) {
			//left child sets victim node to 1, right to 0, thus deffering in the event of simultanious access 
			victim_heap[victim_of(pos)] = is_left(pos);
			//wait until parent interest flag is empty and (sibling not interested or sibling has defered to me)
			while (interest_heap[parent_of(pos)] == 1 ||
				(interest_heap[sibling_of(pos)] == 1 && victim_heap[victim_of(pos)] == is_left(pos))
				) {
				std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
			}
			//once we are promoted we set parent interest to 1, remove sibling interest, then update pos
			interest_heap[parent_of(pos)] = 1;
			interest_heap[pos] = 0;
			pos = parent_of(pos);
		}
	}

	void unlock(int thread_id) {
		interest_heap[0] = 0;
	}

} heap_mutex;

static heap_mutex* initialize_heap_mutex(int num_threads) {
	heap_mutex* tmp = new heap_mutex;

	//heap has enough nodes such that it is perfect and each thread gets a leaf (can optimize this later with new thread entry function, right now it just uses the most convinient leaf)
	int tmp_IHS = 1;
	int tmp_VHS;
	while (tmp_IHS < num_threads) {
		tmp_IHS *= 2;
	}
	tmp_VHS = tmp_IHS - 1;
	tmp_IHS = tmp_IHS * 2 - 1;

	tmp->interest_heap_size = tmp_IHS;
	tmp->victim_heap_size = tmp_VHS;

	bool* tmp_IH = (bool*)calloc(tmp_IHS, sizeof(bool));
	bool* tmp_VH = (bool*)calloc(tmp_VHS, sizeof(bool));

	tmp->victim_heap = tmp_VH;
	tmp->interest_heap = tmp_IH;

	return tmp;
}