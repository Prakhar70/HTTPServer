#include "syncflag.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#include <cassert>
#include <chrono>
#include <Windows.h>

SyncFlag gFlag;
std::atomic<int> gReadCounter{0};
std::atomic<int> gWriteCounter{0};
std::atomic<bool> isWriteInProgress{false};
std::atomic<bool> hasError{false};

void ReaderTask(int id) {
    while (!hasError) {
        {
            SyncReadHold lock(&gFlag);
            // Check that no writer is in progress
            if (isWriteInProgress.load()) {
                std::cerr << " Reader " << id << " entered while writer in progress!" << std::endl;
                hasError = true;
                return;
            }

            gReadCounter++;
            std::cout << "Reader \n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void WriterTask(int id) {
    while (!hasError) {
        {
            SyncWriteHold lock(&gFlag);

            if (isWriteInProgress.exchange(true)) {
                std::cerr << " Writer " << id << " overlap detected!" << std::endl;
                hasError = true;
                return;
            }

            gWriteCounter++;
            std::cout << "Writer " << id << " updated counter to " << gWriteCounter.load() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            isWriteInProgress = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    std::vector<std::thread> threads;

    // Launch 5 reader threads
    for (int i = 0; i < 100; ++i)
        threads.emplace_back(ReaderTask, i);

    // Launch 2 writer threads
    for (int i = 0; i < 60; ++i)
        threads.emplace_back(WriterTask, i);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    bool violationDetected = hasError.load(); 

    hasError = true;

    for (auto& t : threads)
        t.join();

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Read count : " << gReadCounter.load() << std::endl;
    std::cout << "Write count: " << gWriteCounter.load() << std::endl;

    if (!violationDetected)
        std::cout << " No overlapping access detected. Lock is working.\n";
    else
        std::cout << " Lock violation occurred.\n";

    return 0;
}