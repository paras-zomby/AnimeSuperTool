#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include <chrono>
#include <iostream>

/**
 * @namespace progresscpp
 * @brief A namespace for the progress bar utility.
 */
namespace progresscpp
{
    /**
     * @class ProgressBar
     * @brief A class to display a progress bar in the console.
     */
    class ProgressBar
    {
    private:
        unsigned int ticks = 0; ///< The current number of ticks.
        int last_progress = -1; ///< The last recorded progress percentage.
        const std::string message; ///< The message to display with the progress bar.
        const unsigned int total_ticks; ///< The total number of ticks for completion.
        const unsigned int bar_width; ///< The width of the progress bar.
        const char complete_char = '='; ///< The character for the completed part of the bar.
        const char incomplete_char = ' '; ///< The character for the incomplete part of the bar.
        const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now(); ///< The start time of the progress bar.

    public:
        /**
         * @brief Construct a new ProgressBar object.
         * @param message The message to display.
         * @param total The total number of ticks.
         * @param width The width of the progress bar.
         * @param complete The character for the completed part.
         * @param incomplete The character for the incomplete part.
         */
        ProgressBar(const std::string &message, unsigned int total, unsigned int width, char complete, char incomplete) : message{message}, total_ticks{total}, bar_width{width}, complete_char{complete}, incomplete_char{incomplete} {}

        /**
         * @brief Construct a new ProgressBar object with default characters.
         * @param message The message to display.
         * @param total The total number of ticks.
         * @param width The width of the progress bar.
         */
        ProgressBar(const std::string &message, unsigned int total, unsigned int width) : message{message}, total_ticks{total}, bar_width{width} {}

        /**
         * @brief Increment the progress ticks.
         * @return The new number of ticks.
         */
        unsigned int operator++() { return ++ticks; }

        /**
         * @brief Display the progress bar in the console.
         * This function calculates and shows the progress, elapsed time, and estimated time remaining (ETA).
         */
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

        /**
         * @brief Mark the progress as complete and display the final state of the bar.
         */
        void done()
        {
            ticks = total_ticks;
            display();
            std::cout << std::endl;
        }
    };
}

#endif // PROGRESSBAR_HPP