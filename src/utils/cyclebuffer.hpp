#ifndef CYCLEBUFFER_HPP
#define CYCLEBUFFER_HPP

/**
 * @class CycleBuffer
 * @brief A template class for a circular buffer.
 * @tparam T The type of elements in the buffer.
 * @tparam size The maximum size of the buffer.
 */
template <typename T, int size>
class CycleBuffer
{
public:
    /**
     * @brief Construct a new CycleBuffer object.
     * @param interval The interval to advance the tail pointer on update.
     */
    CycleBuffer(int interval) : head_(0), tail_(size - 1), interval_(interval + 1) {};
    /**
     * @brief Destroy the CycleBuffer object.
     */
    ~CycleBuffer() {};

    /**
     * @brief Access an element in the buffer relative to the head.
     * @param index The index of the element to access.
     * @return T& A reference to the element at the specified index.
     */
    T& operator[](int index)
    {
        return buffer[(head_ + index) % size];
    }
    /**
     * @brief Update the buffer's head and tail pointers.
     */
    void update()
    {
        head_ = tail_;
        tail_ = (tail_ + interval_) % size;
    }
    /**
     * @brief Get a reference to the head element of the buffer.
     * @return T& A reference to the head element.
     */
    T& head()
    {
        return buffer[head_];
    }
    /**
     * @brief Get a reference to the tail element of the buffer.
     * @return T& A reference to the tail element.
     */
    T& tail()
    {
        return buffer[tail_];
    }

private:
    T buffer[size];
    int head_;
    int tail_;
    int interval_;
};

#endif // CYCLEBUFFER_HPP