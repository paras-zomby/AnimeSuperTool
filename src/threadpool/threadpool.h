#include <queue>
#include <vector>

#include "net.h"

template <typename TaskParamType>
class ThreadPool
{
private:
    std::vector<ncnn::Thread> threads;
    ncnn::Mutex queue_mutex;
    std::queue<TaskParamType> tasks;
    ncnn::ConditionVariable condition;
    bool stop;

    int num_threads;

public:
    ThreadPool(int num_threads);
    ~ThreadPool();
    

    void start()
    {
        
    }
};

