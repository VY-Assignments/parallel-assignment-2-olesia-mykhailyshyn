#include "ThreadPool.h"
#include "TaskGenerator.h"
#include <iostream>
#include <thread>

int main() {
    ThreadPool pool(3, 1);
    TaskGenerator::GenerateTasks(pool, 5);

    while (!pool.IsEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Main] All tasks are completed." << std::endl;
    return 0;
}