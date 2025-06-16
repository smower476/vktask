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
    std::ofstream log_file("temperature.log");

    collector.register_gauge("sensor_temperature");

    collector.start_writer(log_file);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> temp_dist(20.0, 80.0); 

    while (running) {
        collector.update("sensor_temperature", temp_dist(gen));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    collector.stop();
    return 0;
}

