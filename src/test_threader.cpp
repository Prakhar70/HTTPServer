#include "tthreader.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void MyWorkerFunction(void* pData) {
    const char* msg = static_cast<const char*>(pData);
    std::cout << "[Worker] Task running: " << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[Worker] Task done: " << msg << std::endl;
}

int main() {
    TThreader pool(4); // Create pool of 4 threads
    pool.SetWorkFunc(MyWorkerFunction);

    const char* taskMessages[] = {
        "Task A", "Task B", "Task C", "Task D", "Task E", "Task F"
    };

    for (int i = 0; i < 6; ++i) {
        pool.ASyncExecute((void*)taskMessages[i]);
        std::cout << "[Main] Dispatched: " << taskMessages[i] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "[Main] Waiting for threads to finish..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3)); // Let threads finish

    return 0;
}