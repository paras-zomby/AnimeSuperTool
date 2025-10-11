#include <queue>
#include <vector>

#include "net.h"

/**
 * @class ThreadPool
 * @brief A template class for a generic thread pool.
 * @tparam TaskParamType The type of the task parameter.
 */
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
    /**
     * @brief Construct a new ThreadPool object.
     * @param num_threads The number of threads in the pool.
     */
    ThreadPool(int num_threads) : threads(num_threads), stop(false), num_threads(num_threads) {}
    /**
     * @brief Destroy the ThreadPool object.
     * This will stop all threads and join them.
     */
    ~ThreadPool()
    {
        {
            ncnn::MutexLockGuard lock(queue_mutex);
            stop = true;
        }
        condition.signal();
        for (auto& thread : threads)
        {
            thread.join();
        }
    }
    
    /**
     * @brief Start the thread pool.
     */
    void start()
    {
        
    }
};

