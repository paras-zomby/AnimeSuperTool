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
            time_elapsed /= 1000.0;

            std::cout << message << " [";

            for (size_t i = 0; i < pos; ++i)
                std::cout << complete_char;
            std::cout << ">";
            for (int i = pos + 1; i < bar_width; ++i)
                std::cout << incomplete_char;

            int elapsed_mins = time_elapsed / 60;
            time_elapsed -= elapsed_mins * 60;
            int elapsed_hours = elapsed_mins / 60;
            elapsed_mins -= elapsed_hours * 60;
            int elapsed_days = elapsed_hours / 24;
            elapsed_hours -= elapsed_days * 24;
            
            int eta_mins = time_eta / 60;
            time_eta -= eta_mins * 60;
            int eta_hours = eta_mins / 60;
            eta_mins -= eta_hours * 60;
            int eta_days = eta_hours / 24;
            eta_hours -= eta_days * 24;

            std::cout << "] " << int(progress * 100.0) << "% "
                      << (elapsed_days > 0 ? std::to_string(elapsed_days) + "d" : "")
                      << (elapsed_hours > 0 ? std::to_string(elapsed_hours) + "h" : "")
                      << (elapsed_mins > 0 ? std::to_string(elapsed_mins) + "m" : "")
                      << time_elapsed << "s | ETA: "
                      << (eta_days > 0 ? std::to_string(eta_days) + "d" : "")
                      << (eta_hours > 0 ? std::to_string(eta_hours) + "h" : "")
                      << (eta_mins > 0 ? std::to_string(eta_mins) + "m" : "")
                      << time_eta << "s      \r";
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