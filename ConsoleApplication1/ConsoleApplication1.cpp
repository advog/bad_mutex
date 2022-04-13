#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include "heap_mutex.h"
#include "atomic_mutex.h"
#include "win32_mutex.h"
#include "windows.h"

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

DWORD WINAPI timewaster_win32(LPVOID lpParam) {
	win32_mutex* mu  = (win32_mutex*)lpParam;
	
	//DuplicateHandle(thread_handle, &thread_handle);
	//printf("%d\n", thread_handle);
	//mu->check_if_alive();

	mu->lock();
	//printf("start thread\n");
	std::this_thread::sleep_for(std::chrono::microseconds(100));
	//printf("stop thread\n");
	with++;
	mu->unlock();
	return 0;
}

static int timewaster_std(int thread_id) {
	mu2.lock();
	//printf("start thread: %d\n", thread_id);
	std::this_thread::sleep_for(std::chrono::microseconds(100));
	//printf("stop thread: %d\n", thread_id);
	with++;
	mu2.unlock();
	return 0;
}

int main()
{
	const int N = 1024;

	//WIN32 MUTEX INITIALIZATION
	DWORD   thread_ids[N];
	HANDLE  thread_handles[N];
	DWORD junkholder;

	win32_mutex* mu4 = new win32_mutex;
	mu4->initialize(1024);
	
	//////////////////////////C++ STDLIB MUTEX///////////////////////////////
	auto start2 = std::chrono::high_resolution_clock::now();

	std::vector<std::thread> ThreadVector2;
	for (int i = 0; i < N; i++){ThreadVector2.push_back(std::thread(timewaster_std, i));}
	for (int i = 0; i < N; i++){ThreadVector2[i].join();}

	auto stop2 = std::chrono::high_resolution_clock::now();
	auto duration2= std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
	printf("std_mutex duration: %d\n", duration2.count());

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
	


	auto start3 = std::chrono::high_resolution_clock::now();
	

	std::vector<std::thread> ThreadVector3;
	printf("mutex initialized  ");
	for (int i = 0; i < N; i++) { ThreadVector3.push_back(std::thread(timewaster_atomic, &mu, i)); }
	printf("threads started  ");
	for (int i = 0; i < N; i++) { ThreadVector3[i].join(); }

	auto stop3 = std::chrono::high_resolution_clock::now();
	auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
	std::cout << duration3.count() << "\n";
	
	*/


	////////////////WIN 32 MUTEX//////////////////
	auto start3 = std::chrono::high_resolution_clock::now();
	for (uint32_t i = 0; i < N; i++) {

		thread_handles[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			timewaster_win32,       // thread function name
			(LPVOID)mu4,          // argument to thread function
			0,                      // use default creation flags 
			&junkholder);   // returns the thread identifier 
	}

	for (int i = 0; i < N; i++)
	{
		WaitForSingleObject(thread_handles[i], INFINITE);
		CloseHandle(thread_handles[i]);
	}

	auto stop3 = std::chrono::high_resolution_clock::now();
	auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
	printf("win32_mutex duration: %d\n", duration3.count());
}
