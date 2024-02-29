/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _BENCHLET_HPP
#define _BENCHLET_HPP

#define __STDC_LIMIT_MACROS 1
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(Darwin)
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#elif defined(__linux__)
#   include <time.h>
#elif defined(WIN32) || defined(_WIN32)
#else
#   error "Must define Darwin or __linux__ or WIN32"
#endif /* platform includes */

#include <iostream>
#include <vector>

#define DEFAULT_ITERATIONS 1
#define DEFAULT_ITERATIONS_STRING "1"

#define DEFAULT_BATCHES 1
#define DEFAULT_BATCHES_STRING "1"

#define DEFAULT_RETAIN_STATS false

class Benchmark
{
public:
    enum ConfigVariable
    {
        ITERATIONS,
        BATCHES,
        RETAIN_STATS
    };

    struct Config
    {
        ConfigVariable key;
        const char *value;
    };

    virtual void setUp() {};
    virtual void tearDown() {};
    virtual uint64_t run(int iterations) = 0;

    void name(const char *name) { name_ = name; };
    const char *name() const { return name_; };

    void runName(const char *runName) { runName_ = runName; };
    const char *runName() const { return runName_; };

    void iterations(const unsigned int i) { iterations_ = i; };
    unsigned int iterations() const { return iterations_; };

    void batches(const unsigned int i) { batches_ = i; };
    unsigned int batches() const { return batches_; };

    void config(const struct Config *cfg, unsigned int numConfigs) { config_ = cfg; numConfigs_ = numConfigs; };
    const struct Config *config() const { return config_; };
    unsigned int numConfigs() const { return numConfigs_; };

    void stats(uint64_t *stats) { stats_ = stats; };
    uint64_t *stats() { return stats_; };

    void retainStats(bool retain) { retainStats_ = retain; };
    bool retainStats() const { return retainStats_; };
private:
    const char *name_ = nullptr;
    const char *runName_ = nullptr;
    unsigned int iterations_ = 0;
    unsigned int batches_ = 0;
    const struct Config *config_ = nullptr;
    unsigned int numConfigs_ = 0;
    uint64_t *stats_ = nullptr;
    bool retainStats_ = false;
    // save start time, etc.
};

class BenchmarkRunner
{
public:
    static Benchmark *registerBenchmark(const char *name, const char *runName, Benchmark *impl, struct Benchmark::Config *cfg, int numCfgs)
    {
        impl->name(name);
        impl->runName(runName);
        impl->config(cfg, numCfgs);
        impl->iterations(DEFAULT_ITERATIONS);
        impl->batches(DEFAULT_BATCHES);
        impl->retainStats(DEFAULT_RETAIN_STATS);
        for (int i = 0, max = numCfgs; i < max; i++)
        {
            if (cfg[i].key == Benchmark::ITERATIONS)
            {
                impl->iterations(atoi(cfg[i].value));
            }
            else if (cfg[i].key == Benchmark::BATCHES)
            {
                impl->batches(atoi(cfg[i].value));
            }
            else if (cfg[i].key == Benchmark::RETAIN_STATS)
            {
                if (strcmp(cfg[i].value, "true") == 0)
                {
                    impl->retainStats(true);
                }
                else if (strcmp(cfg[i].value, "false") == 0)
                {
                    impl->retainStats(false);
                }
            }
        }
        table().push_back(impl);
        std::cout << "Registering " << name << " run \"" << runName << "\" total iterations " << impl->iterations() * impl->batches() << std::endl;
        return impl;
    };

    static void run()
    {
        for (std::vector<Benchmark *>::iterator it = table().begin(); it != table().end(); ++it)
        {
            Benchmark *benchmark = *it;
            uint64_t elapsedNanos;
            double nanospop, opspsec;
            uint64_t *stats = new uint64_t[benchmark->batches()];
            uint64_t total = 0;

            std::cout << "Running benchmark " << benchmark->name() << "." << benchmark->runName() << ". ";
            std::cout << benchmark->iterations() << " iterations X " << benchmark->batches() << " batches. " << std::endl;
            benchmark->setUp();
            for (int i = 0, max_i = benchmark->batches(); i < max_i; i++)
            {
                elapsedNanos = benchmark->run(benchmark->iterations());
                nanospop = (double)elapsedNanos / (double)benchmark->iterations();
                opspsec = 1000000000.0 / nanospop;
                stats[i] = elapsedNanos;
                total += elapsedNanos;
                std::cout << " Elapsed " << elapsedNanos << " nanoseconds. ";
                std::cout << opspsec/1000.0 << " Kops/sec. ";
                std::cout << nanospop << " nanos/op. ";
                std::cout << std::endl;
            }
            double elapsedPerBatch = (double)total / (double)benchmark->batches();
            double elapsedPerIteration = elapsedPerBatch / (double)benchmark->iterations();
            double throughputKopsps = 1000000.0 / elapsedPerIteration;
            std::cout << " Avg elapsed/batch " << elapsedPerBatch << " nanoseconds" << std::endl;
            std::cout << " Throughput " << throughputKopsps << " Kops/sec." << std::endl;
            std::cout << " Avg nanos/op " << elapsedPerIteration << " nanos/op" << std::endl;
            benchmark->stats(stats);
            benchmark->tearDown();
            total = 0;
            if (!benchmark->retainStats())
            {
                delete[] stats;
            }
        }
    };

    static std::vector<Benchmark *> &table()
    {
        static std::vector<Benchmark *> table;
        return table;
    };

#if defined(Darwin)
    static uint64_t currentTimestamp(void)
    {
        return mach_absolute_time();
    };

    // inspired from https://developer.apple.com/library/mac/qa/qa1398/_index.html
    static uint64_t elapsedNanoseconds(uint64_t start_timestamp, uint64_t end_timestamp)
    {
        static mach_timebase_info_data_t timebaseInfo;

        if (0 == timebaseInfo.denom)
        {
            (void)mach_timebase_info(&timebaseInfo);
        }
        return (end_timestamp - start_timestamp) * timebaseInfo.numer / timebaseInfo.denom;
    };
#elif defined(__linux__)
    static uint64_t currentTimestamp()
    {
        struct timespec ts = {};

        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * 1000000000 + ts.tv_nsec;
    };

    static uint64_t elapsedNanoseconds(uint64_t start_timestamp, uint64_t end_timestamp)
    {
        return end_timestamp - start_timestamp;
    };
#elif defined(WIN32) || defined(_WIN32)
    static uint64_t currentTimestamp(void);
    static uint64_t elapsedNanoseconds(uint64_t start_timestamp, uint64_t end_timestamp)
    {
        return end_timestamp - start_timestamp;
    }
#endif /* platform high resolution time */
};

template <typename C>
uint64_t runBenchmark(C *obj, int iterations)
{
    uint64_t start = BenchmarkRunner::currentTimestamp();
    for (int i = 0; i < iterations; i++)
    {
        obj->benchmarkBody();
    }
    uint64_t end = BenchmarkRunner::currentTimestamp();

    return BenchmarkRunner::elapsedNanoseconds(start, end);
}

#define BENCHMARK_CLASS_NAME(x,y) x##y

#define BENCHMARK_CONFIG(c,r,i) \
    class BENCHMARK_CLASS_NAME(c,r) : public c { \
    public: \
      BENCHMARK_CLASS_NAME(c,r)() {}; \
      virtual uint64_t run(int iterations) { return runBenchmark<BENCHMARK_CLASS_NAME(c,r)>(this, iterations); }; \
      void benchmarkBody(); \
    private: \
      static Benchmark *instance_; \
    };                            \
    Benchmark *BENCHMARK_CLASS_NAME(c,r)::instance_ = BenchmarkRunner::registerBenchmark(#c, #r, new BENCHMARK_CLASS_NAME(c,r)(), i, sizeof(i)/sizeof(Benchmark::Config)); \
    void BENCHMARK_CLASS_NAME(c,r)::benchmarkBody()

#define BENCHMARK_ITERATIONS(c,r,i) \
    struct Benchmark::Config c ## r ## _cfg[] = {{Benchmark::ITERATIONS, #i},{Benchmark::BATCHES, DEFAULT_BATCHES_STRING}}; \
    BENCHMARK_CONFIG(c,r, c ## r ## _cfg)

#define BENCHMARK(c,r) \
    struct Benchmark::Config c ## r ## _cfg[] = {{Benchmark::ITERATIONS, DEFAULT_ITERATIONS_STRING},{Benchmark::BATCHES, DEFAULT_BATCHES_STRING}}; \
    BENCHMARK_CONFIG(c,r, c ## r ## _cfg)

#endif /* _BENCHLET_HPP */
