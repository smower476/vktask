#include "metrics_lib.h"
#include <fstream>
#include <random>
#include <csignal>

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    MetricsCollector collector;
    std::ofstream log_file("errors.log");

    collector.register_counter("error_events");

    collector.start_writer(log_file);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution error_dist(0.1); 

    while (running) {
        if (error_dist(gen)) {
            collector.update("error_events", 1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    collector.stop();
    return 0;
}

