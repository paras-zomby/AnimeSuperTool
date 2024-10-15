#ifndef CYCLEBUFFER_HPP
#define CYCLEBUFFER_HPP

template <typename T, int size>
class CycleBuffer
{
public:
    CycleBuffer(int interval) : head_(0), tail_(size - 1), interval_(interval + 1) {};
    ~CycleBuffer() {};

    T& operator[](int index)
    {
        return buffer[(head_ + index) % size];
    }
    void update()
    {
        head_ = tail_;
        tail_ = (tail_ + interval_) % size;
    }
    T& head()
    {
        return buffer[head_];
    }
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