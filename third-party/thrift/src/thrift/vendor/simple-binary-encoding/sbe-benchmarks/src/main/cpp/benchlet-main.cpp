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

int main(int argc, char **argv)
{
    BenchmarkRunner::run();
    return 0;
}

#ifdef _WIN32
#include <Windows.h>
uint64_t BenchmarkRunner::currentTimestamp()
{
    static LARGE_INTEGER freq;
    static int first = 1;
    LARGE_INTEGER counter;

    ::QueryPerformanceCounter(&counter);
    if (1 == first)
    {
        ::QueryPerformanceFrequency(&freq);
        first = 0;
    }
    return (1000000000 * counter.QuadPart) / freq.QuadPart;
}

#endif