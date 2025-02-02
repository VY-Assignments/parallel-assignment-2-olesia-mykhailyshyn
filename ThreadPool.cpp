#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(int firstQueueWorkers, int secondQueueWorkers)
        : stop(false), firstPriorityQueue(TaskComparator), activeTasks(0) {
    for (int i = 0; i < firstQueueWorkers; ++i) {
        workersFirstQueue.emplace_back(&ThreadPool::WorkerFirstPriority, this, i + 1);
    }
    workerSecondQueue = std::thread(&ThreadPool::WorkerSecondQueue, this, firstQueueWorkers + 1);
}

ThreadPool::~ThreadPool() {
    stop = true;
    firstPriorityQueueCV.notify_all();
    secondQueueCV.notify_all();

    for (auto& worker : workersFirstQueue) {
        worker.join();
    }
    workerSecondQueue.join();
}

bool ThreadPool::IsEmpty() {
    std::lock_guard<std::mutex> lock1(firstPriorityQueueMutex);
    std::lock_guard<std::mutex> lock2(secondQueueMutex);
    return firstPriorityQueue.empty() && secondQueue.empty() && activeTasks == 0;
}

bool ThreadPool::TaskComparator(const Task& a, const Task& b) {
    return std::get<0>(a) > std::get<0>(b);
}

void ThreadPool::AddTask(int duration, int id) {
    if (std::rand() % 100 < 40) {
        duration *= 3;
    }
    std::cout << "[Task Created] Task " << id << " | Duration: " << duration << " seconds" << std::endl;

    auto creationTime = std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::mutex> lock(firstPriorityQueueMutex);
        firstPriorityQueue.push({duration, id, [duration, id, this]() {
            activeTasks++;
            auto startExecTime = std::chrono::high_resolution_clock::now();
            std::cout << "[Execution Start] Task " << id << " started." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            auto endExecTime = std::chrono::high_resolution_clock::now();
            std::cout << "[Execution Done] Task " << id << " | Expected: " << duration
                      << " sec | Actual Time: "
                      << std::chrono::duration_cast<std::chrono::seconds>(endExecTime - startExecTime).count()
                      << " sec" << std::endl;
            activeTasks--;
        }, creationTime});
        taskStartTimes[id] = creationTime;
    }
    firstPriorityQueueCV.notify_one();
}

void ThreadPool::WorkerFirstPriority(int workerId) {
    while (!stop) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(firstPriorityQueueMutex);
            firstPriorityQueueCV.wait(lock, [this] { return !firstPriorityQueue.empty() || stop; });
            if (stop && firstPriorityQueue.empty()) {
                return;
            }
            task = firstPriorityQueue.top();
            firstPriorityQueue.pop();
        }

        auto dequeueTime = std::chrono::high_resolution_clock::now();
        std::cout << "[Worker " << workerId << "] Took Task " << std::get<1>(task) << " from first queue" << std::endl;
        std::get<2>(task)();
        auto endTime = std::chrono::high_resolution_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(endTime - dequeueTime).count() > 2 * std::get<0>(task)) {
            std::lock_guard<std::mutex> lock(secondQueueMutex);
            secondQueue.push({std::get<1>(task), std::get<2>(task), dequeueTime});
            secondQueueCV.notify_one();
            std::cout << "[Worker " << workerId << "] Task " << std::get<1>(task) << " moved to second queue" << std::endl;
        }
        else {
            std::cout << "[Worker " << workerId << "] Completed Task " << std::get<1>(task) << " in first queue" << std::endl;
        }
    }
}

void ThreadPool::WorkerSecondQueue(int workerId) {
    while (!stop) {
        std::tuple<int, std::function<void()>, std::chrono::high_resolution_clock::time_point> task;
        {
            std::unique_lock<std::mutex> lock(secondQueueMutex);
            secondQueueCV.wait(lock, [this] {
                return !secondQueue.empty() || stop; });
            if (stop && secondQueue.empty()) {
                return;
            }
            task = secondQueue.front();
            secondQueue.pop();
        }
        std::cout << "[Worker " << workerId << "] Executing Task " << std::get<0>(task) << " from second queue" << std::endl;
        std::get<1>(task)();
    }
}