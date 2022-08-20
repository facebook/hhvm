/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

#include <thrift/lib/cpp/concurrency/test/ThreadFactoryTests.h>
#include <thrift/lib/cpp/concurrency/test/TimerManagerTests.h>

using namespace apache::thrift::concurrency;
using namespace apache::thrift::concurrency::test;

int main(int argc, char** argv) {
  std::string arg;

  std::vector<std::string> args(argc - 1 > 1 ? argc - 1 : 1);

  args[0] = "all";

  for (int ix = 1; ix < argc; ix++) {
    args[ix - 1] = std::string(argv[ix]);
  }

  bool runAll = args[0] == "all";

  if (runAll || args[0] == "thread-factory") {
    ThreadFactoryTests threadFactoryTests;

    std::cout << "ThreadFactory tests..." << std::endl;

    size_t count = 1000;
    size_t floodLoops = 1;
    size_t floodCount = 100000;

    // DCHECK(threadFactoryTests.helloWorldTest());
    DCHECK(threadFactoryTests.getCurrentThreadIdTest());

    std::cout << "\t\tThreadFactory reap N threads test: N = " << count
              << std::endl;

    DCHECK(threadFactoryTests.reapNThreads(count));

    std::cout << "\t\tThreadFactory floodN threads test: N = " << floodCount
              << std::endl;

    DCHECK(threadFactoryTests.floodNTest(floodLoops, floodCount));

    std::cout << "\t\tThreadFactory synchronous start test" << std::endl;

    DCHECK(threadFactoryTests.synchStartTest());

    std::cout << "\t\tThreadFactory condition variable timeout test"
              << std::endl;

    DCHECK(threadFactoryTests.conditionVariableTimeoutTest());

    std::cout << "\t\tInitThreadFactory test" << std::endl;

    DCHECK(threadFactoryTests.initThreadFactoryTest());
  }

  if (runAll || args[0] == "util") {
    std::cout << "Util tests..." << std::endl;

    std::cout << "\t\tUtil minimum time" << std::endl;

    int64_t time00 = Util::currentTime();
    int64_t time01 = Util::currentTime();

    std::cout << "\t\t\tMinimum time: " << time01 - time00 << "ms" << std::endl;

    time00 = Util::currentTime();
    time01 = time00;
    size_t count = 0;

    while (time01 < time00 + 10) {
      count++;
      time01 = Util::currentTime();
    }

    std::cout << "\t\t\tscall per ms: " << count / (time01 - time00)
              << std::endl;
  }

  if (runAll || args[0] == "timer-manager") {
    std::cout << "TimerManager tests..." << std::endl;

    std::cout << "\t\tTimerManager test00" << std::endl;

    TimerManagerTests timerManagerTests;

    DCHECK(timerManagerTests.test00());
  }
}
