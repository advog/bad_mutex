#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include "heap_mutex.h"
#include "atomic_mutex.h"

int with = 0;
int without = 0;

std::mutex mu2;


static int timewaster(int thread_id, heap_mutex* mu) {
	mu->lock(thread_id);
	//printf("start thread: %d\n", thread_id);
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	//printf("stop thread: %d\n", thread_id);
	with++;
	mu->unlock(thread_id);
	return thread_id;
}

static int timewaster_atomic(atomic_mutex* mu, int thread_id) {
	mu->lock(thread_id);
	//printf("start thread: %d\n", thread_id);
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	//printf("stop thread: %d\n", thread_id);
	with++;
	mu->unlock(thread_id);
	return thread_id;
}

static int timewaster_std(int thread_id) {
	mu2.lock();
	//printf("start thread: %d\n", thread_id);
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	//printf("stop thread: %d\n", thread_id);
	with++;
	mu2.unlock();
	return thread_id;
}

int main()
{
	int N = 1024;

	

	atomic_mutex mu;
	mu.initialize(N);
	
	heap_mutex* mu3 = initialize_heap_mutex(N);
	
	auto start2 = std::chrono::high_resolution_clock::now();

	std::vector<std::thread> ThreadVector2;
	printf("mutex initialized  ");
	for (int i = 0; i < N; i++){ThreadVector2.push_back(std::thread(timewaster_std, i));}
	printf("threads started  ");
	for (int i = 0; i < N; i++){ThreadVector2[i].join();}

	auto stop2 = std::chrono::high_resolution_clock::now();
	auto duration2= std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
	std::cout << duration2.count() << "\n";

	/*
	auto start1 = std::chrono::high_resolution_clock::now();
	
	std::vector<std::thread> ThreadVector;
	printf("mutex initialized  ");
	for (int i = 0; i < N; i++){ThreadVector.push_back(std::thread(timewaster, i, mu3));}
	printf("threads started  ");
	for (int i = 0; i < N; i++){ThreadVector[i].join();}

	auto stop1 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
	std::cout << duration.count() << "\n";
	*/

	auto start3 = std::chrono::high_resolution_clock::now();
	

	std::vector<std::thread> ThreadVector3;
	printf("mutex initialized  ");
	for (int i = 0; i < N; i++) { ThreadVector3.push_back(std::thread(timewaster_atomic, &mu, i)); }
	printf("threads started  ");
	for (int i = 0; i < N; i++) { ThreadVector3[i].join(); }

	auto stop3 = std::chrono::high_resolution_clock::now();
	auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
	std::cout << duration3.count() << "\n";
	
}
