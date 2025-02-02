#pragma once
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <functional>
#include <chrono>

class ThreadPool {
public:
    ThreadPool(int firstQueueWorkers, int secondQueueWorkers);
    ~ThreadPool();

    void AddTask(int duration, int id);
    bool IsEmpty();

private:
    using Task = std::tuple<int, int, std::function<void()>, std::chrono::high_resolution_clock::time_point>;

    static bool TaskComparator(const Task& a, const Task& b);

    std::priority_queue<Task, std::vector<Task>, decltype(&TaskComparator)> firstPriorityQueue;
    std::queue<std::tuple<int, std::function<void()>, std::chrono::high_resolution_clock::time_point>> secondQueue;

    std::mutex firstPriorityQueueMutex, secondQueueMutex;
    std::condition_variable firstPriorityQueueCV, secondQueueCV;
    std::vector<std::thread> workersFirstQueue;
    std::thread workerSecondQueue;

    std::atomic<bool> stop;
    std::atomic<int> activeTasks;
    std::map<int, std::chrono::high_resolution_clock::time_point> taskStartTimes;

    void WorkerFirstPriority(int workerId);
    void WorkerSecondQueue(int workerId);
};