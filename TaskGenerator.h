#pragma once
#include "ThreadPool.h"

class TaskGenerator {
public:
    static void GenerateTasks(ThreadPool& pool, int taskCount);
};