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
#include "SbeMarketDataCodecBench.h"

#define MAX_MD_BUFFER (1000*1000)

class SbeMarketDataBench : public Benchmark
{
public:
    void setUp() override
    {
        buffer_ = new char[MAX_MD_BUFFER];
        bench_.runEncode(buffer_, MAX_MD_BUFFER);  // set buffer up for decoding runs
    };

    void tearDown() override
    {
        delete[] buffer_;
    };

    SbeMarketDataCodecBench bench_;
    char *buffer_ = nullptr;
};

static struct Benchmark::Config cfg[] =
{
    { Benchmark::ITERATIONS, "10000000" },
    { Benchmark::BATCHES, "20" }
};

BENCHMARK_CONFIG(SbeMarketDataBench, RunSingleEncode, cfg)
{
    bench_.runEncode(buffer_, MAX_MD_BUFFER);
}

BENCHMARK_CONFIG(SbeMarketDataBench, RunSingleDecode, cfg)
{
    bench_.runDecode(buffer_, MAX_MD_BUFFER);
}

BENCHMARK_CONFIG(SbeMarketDataBench, RunSingleEncodeAndDecode, cfg)
{
    bench_.runEncodeAndDecode(buffer_, MAX_MD_BUFFER);
}
