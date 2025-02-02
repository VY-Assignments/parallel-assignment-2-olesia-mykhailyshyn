#include "TaskGenerator.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

void TaskGenerator::GenerateTasks(ThreadPool& pool, int taskCount) {
    std::srand(std::time(nullptr));
    for (int i = 0; i < taskCount; ++i) {
        int duration = 5 + std::rand() % 6;
        if (std::rand() % 100 < 40) {
            duration *= 3;
        }
        pool.AddTask(duration, i + 1);
    }
}