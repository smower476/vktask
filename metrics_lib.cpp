#include "metrics_lib.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace std::chrono;

void CounterMetric::write(std::ostream& out) const {
    out << '"' << name << "\" " << value;
}

void GaugeMetric::write(std::ostream& out) const {
    out << '"' << name << "\" " << value;
}

MetricsCollector::MetricsCollector() = default;
MetricsCollector::~MetricsCollector() { stop(); }

void MetricsCollector::register_counter(const std::string& name) {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    for (auto& buffer : buffers) {
        buffer[name] = std::make_unique<CounterMetric>(name);
    }
}

void MetricsCollector::register_gauge(const std::string& name) {
    std::lock_guard<std::mutex> lock(metrics_mutex);
    for (auto& buffer : buffers) {
        buffer[name] = std::make_unique<GaugeMetric>(name);
    }
}

void MetricsCollector::update(const std::string& name, int value) {
    int buf = active_buffer.load();
    std::lock_guard<std::mutex> lock(metrics_mutex);
    auto it = buffers[buf].find(name);
    if (it != buffers[buf].end()) {
        it->second->update(value);
    }
}

void MetricsCollector::update(const std::string& name, double value) {
    int buf = active_buffer.load();
    std::lock_guard<std::mutex> lock(metrics_mutex);
    auto it = buffers[buf].find(name);
    if (it != buffers[buf].end()) {
        it->second->update(value);
    }
}

void MetricsCollector::start_writer(std::ostream& out, unsigned interval_ms) {
    running = true;
    writer_thread = std::thread(&MetricsCollector::writer_loop, this, std::ref(out), interval_ms);
}

void MetricsCollector::stop() {
    running = false;
    if (writer_thread.joinable()) {
        writer_thread.join();
    }
}

void MetricsCollector::writer_loop(std::ostream& out, unsigned interval_ms) {
    while (running) {
        auto start = system_clock::now();
        
        int buf_to_write = active_buffer.load();
        int buf_to_update = 1 - buf_to_write;

        {
            std::lock_guard<std::mutex> lock(metrics_mutex);
            active_buffer.store(buf_to_update);
        }

        auto& write_metrics = buffers[buf_to_write];

        bool non_empty = false;
        for (const auto& [_, metric] : write_metrics) {
            non_empty = true;
            break;
        }

        if (non_empty) {
            auto now = system_clock::now();
            auto t = system_clock::to_time_t(now);
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

            out << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
            out << '.' << std::setfill('0') << std::setw(3) << ms.count();

            for (auto& [_, metric] : write_metrics) {
                out << ' ';
                metric->write(out);
                metric->reset();
            }
            out << std::endl;
        }

        std::this_thread::sleep_until(start + milliseconds(interval_ms));
    }
}

