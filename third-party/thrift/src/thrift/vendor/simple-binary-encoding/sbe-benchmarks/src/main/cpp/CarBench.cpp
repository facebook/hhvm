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
#include "benchlet.h"
#include "SbeCarCodecBench.h"

#define MAX_CAR_BUFFER (1000 * 1000)
#define MAX_N 10

class SbeCarBench : public Benchmark
{
public:
    void setUp() override
    {
        buffer_ = new char[MAX_CAR_BUFFER];
        bench_.runEncode(buffer_, MAX_N, MAX_CAR_BUFFER);  // set buffer up for decoding runs
        std::cout << "MAX N = " << MAX_N << " [for Multiple runs]" << std::endl;
    };

    void tearDown() override
    {
        delete[] buffer_;
    };

    SbeCarCodecBench bench_;
    char *buffer_ = nullptr;
};

static struct Benchmark::Config cfg[] =
{
    { Benchmark::ITERATIONS, "1000000" },
    { Benchmark::BATCHES, "20" }
};

BENCHMARK_CONFIG(SbeCarBench, RunSingleEncode, cfg)
{
    bench_.runEncode(buffer_, MAX_CAR_BUFFER);
}

BENCHMARK_CONFIG(SbeCarBench, RunSingleDecode, cfg)
{
    bench_.runDecode(buffer_, MAX_CAR_BUFFER);
}

BENCHMARK_CONFIG(SbeCarBench, RunSingleEncodeAndDecode, cfg)
{
    bench_.runEncodeAndDecode(buffer_, MAX_CAR_BUFFER);
}
