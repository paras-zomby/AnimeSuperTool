#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include <chrono>
#include <iostream>

namespace progresscpp
{
    class ProgressBar
    {
    private:
        unsigned int ticks = 0;
        int last_progress = -1;
        const std::string message;
        const unsigned int total_ticks;
        const unsigned int bar_width;
        const char complete_char = '=';
        const char incomplete_char = ' ';
        const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    public:
        ProgressBar(const std::string &message, unsigned int total, unsigned int width, char complete, char incomplete) : message{message}, total_ticks{total}, bar_width{width}, complete_char{complete}, incomplete_char{incomplete} {}

        ProgressBar(const std::string &message, unsigned int total, unsigned int width) : message{message}, total_ticks{total}, bar_width{width} {}

        unsigned int operator++() { return ++ticks; }

        void display()
        {
            float progress = (float)ticks / total_ticks;

            if (int(progress * 100.0) == last_progress)
                return;
            
            last_progress = int(progress * 100.0);

            int pos = (int)(bar_width * progress);

            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

            auto time_eta = int(time_elapsed / progress) - time_elapsed;
            time_eta /= 1000.0;

            std::cout << message << " [";

            for (size_t i = 0; i < pos; ++i)
                std::cout << complete_char;
            std::cout << ">";
            for (int i = pos + 1; i < bar_width; ++i)
                std::cout << incomplete_char;
            
            int time_mins = time_eta / 60;
            time_eta -= time_mins * 60;
            int time_hours = time_mins / 60;
            time_mins -= time_hours * 60;
            int time_days = time_hours / 24;
            time_hours -= time_days * 24;

            std::cout << "] " << int(progress * 100.0) << "% "
                      << float(time_elapsed) / 1000.0 << "s | ETA: "
                      << (time_days > 0 ? std::to_string(time_days) + "d" : "")
                      << (time_hours > 0 ? std::to_string(time_hours) + "h" : "")
                      << (time_mins > 0 ? std::to_string(time_mins) + "m" : "")
                      << float(time_eta) << "s      \r";
            std::cout.flush();
        }

        void done()
        {
            ticks = total_ticks;
            display();
            std::cout << std::endl;
        }
    };
}

#endif // PROGRESSBAR_HPP