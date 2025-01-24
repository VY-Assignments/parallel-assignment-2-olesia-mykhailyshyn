#include <queue>
#include <cstdlib> //for rand
#include <ctime> //time in srand
#include <thread>
#include <chrono> //for time delaying
#include <iostream>
#include <functional> //for doind queue of functions
#include <mutex>

std::mutex tasksGeneratorMutex;
std::queue<std::pair<int, std::function<void()>>> tasksQueue;

void Generator() {
    int taskDuration = 5 + std::rand() % 6;
    std::cout << "\nProceeding the task will take " << taskDuration << "seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(taskDuration));
}

void TaskGenerator(int threadId, std::queue<std::pair<int, std::function<void()>>>& taskQueue) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(tasksGeneratorMutex);
            if (taskQueue.size() >= 20) {
                break;
            }
            taskQueue.push({threadId, []() { Generator(); }});
            std::cout << "The task was generated and added to taskQueue from thread " << threadId << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::srand(std::time(nullptr));

    std::thread generatorProcess1(TaskGenerator, 1, std::ref(tasksQueue));
    std::thread generatorProcess2(TaskGenerator, 2, std::ref(tasksQueue));

    generatorProcess1.join();
    generatorProcess2.join();

    std::cout << "\nThere are " << tasksQueue.size() << " generated tasks" << std::endl;
}