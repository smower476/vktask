#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <ostream>

class IMetric {
public:
    virtual ~IMetric() = default;
    virtual void update(int value) = 0;
    virtual void update(double value) = 0;
    virtual void write(std::ostream& out) const = 0;
    virtual void reset() = 0;
    virtual std::unique_ptr<IMetric> clone() const = 0;
};

class CounterMetric : public IMetric {
    std::string name;
    int value = 0;
public:
    CounterMetric(const std::string& n) : name(n) {}
    void update(int v) override { value += v; }
    void update(double) override {}
    void write(std::ostream& out) const override;
    void reset() override { value = 0; }
    std::unique_ptr<IMetric> clone() const override {
        return std::make_unique<CounterMetric>(*this);
    }
};

class GaugeMetric : public IMetric {
    std::string name;
    double value = 0.0;
public:
    GaugeMetric(const std::string& n) : name(n) {}
    void update(int v) override { value = v; }
    void update(double v) override { value = v; }
    void write(std::ostream& out) const override;
    void reset() override { value = 0.0; }
    std::unique_ptr<IMetric> clone() const override {
        return std::make_unique<GaugeMetric>(*this);
    }
};

class MetricsCollector {
    std::unordered_map<std::string, std::unique_ptr<IMetric>> buffers[2];
    std::atomic<int> active_buffer{0};
    std::mutex metrics_mutex;

    std::thread writer_thread;
    std::atomic<bool> running{false};

    void writer_loop(std::ostream& out, unsigned interval_ms);
public:
    MetricsCollector();
    ~MetricsCollector();

    void register_counter(const std::string& name);
    void register_gauge(const std::string& name);

    void update(const std::string& name, int value);
    void update(const std::string& name, double value);

    void start_writer(std::ostream& out, unsigned interval_ms = 1000);
    void stop();
};

