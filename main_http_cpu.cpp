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
    std::ofstream log_file("http_cpu.log");

    collector.register_counter("http_requests");
    collector.register_gauge("cpu_usage");

    collector.start_writer(log_file);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> requests_dist(0, 100);
    std::uniform_real_distribution<> cpu_dist(0.0, 4.0);

    while (running) {
        collector.update("http_requests", requests_dist(gen));
        collector.update("cpu_usage", cpu_dist(gen));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    collector.stop();
    return 0;
}

