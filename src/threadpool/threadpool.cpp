#include "threadpool.h"

template <typename TaskParamType>
ThreadPool<TaskParamType>::ThreadPool(int num_threads) : num_threads(num_threads)
{
}

template <typename TaskParamType>
ThreadPool<TaskParamType>::~ThreadPool()
{
}