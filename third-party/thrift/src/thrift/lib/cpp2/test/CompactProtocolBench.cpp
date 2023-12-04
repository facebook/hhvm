/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

#include <thrift/lib/cpp2/test/gen-cpp2/CompactProtocolBenchData_types.h>

#include <iostream>
#include <vector>

#include <fmt/core.h>
#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/Optional.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace ::cpp2;

const size_t kMultExp = 0;

Shallow makeShallow() {
  Shallow data;
  data.var1() = 750ll;
  data.var2() = 750ll << 22;
  data.var2() = 8017696980862921917;
  data.var3() = 8507923373123406843;
  data.var4() = 5834263759460133508;
  data.var5() = 6077406608576733812;
  data.var6() = 3842838073381369466;
  data.var7() = 2005442274692652768;
  data.var8() = 4637177433877214444;
  data.var9() = 2020473258567854822;
  data.var10() = 8084493851963638918;
  data.var11() = 8217038917532557511;
  data.var12() = 1249185632320494101;
  data.var13() = 4918664689356915587;
  data.var14() = 5925259068042961005;
  data.var15() = 8151593475499675445;
  data.var16() = 4976640777863928983;
  data.var17() = 293642974317649048;
  data.var18() = 6756909131245608540;
  data.var19() = 3502392819420581197;
  data.var20() = 4971435190867297895;
  return data;
}

Deep makeDeep(size_t triplesz) {
  Deep data;
  for (size_t i = 0; i < triplesz; ++i) {
    Deep1 data1;
    for (size_t j = 0; j < triplesz; ++j) {
      Deep2 data2;
      for (size_t k = 0; k < triplesz; ++k) {
        data2.datas_ref()->push_back(fmt::format("omg[{}, {}, {}]", i, j, k));
      }
      data1.deeps_ref()->push_back(std::move(data2));
    }
    data.deeps_ref()->push_back(std::move(data1));
  }
  return data;
}

BENCHMARK(CompactProtocolReader_ctor, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  vector<Optional<CompactProtocolReader>> protos(iters);
  braces.dismiss();
  while (iters--) {
    protos[iters].emplace();
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolReader_dtor, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  vector<Optional<CompactProtocolReader>> protos(iters);
  for (auto& proto : protos) {
    proto.emplace();
  }
  braces.dismiss();
  while (iters--) {
    protos[iters].reset();
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolWriter_ctor, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  vector<Optional<CompactProtocolWriter>> protos(iters);
  braces.dismiss();
  while (iters--) {
    protos[iters].emplace();
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolWriter_dtor, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  vector<Optional<CompactProtocolWriter>> protos(iters);
  for (auto& proto : protos) {
    proto.emplace();
  }
  braces.dismiss();
  while (iters--) {
    protos[iters].reset();
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolReader_deserialize_empty, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Empty data;
  CompactSerializer ser;
  IOBufQueue bufq;
  ser.serialize(data, &bufq);
  auto buf = bufq.move();
  braces.dismiss();
  while (iters--) {
    CompactSerializer s;
    Empty empty;
    s.deserialize(buf.get(), empty);
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolWriter_serialize_empty, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Empty data;
  braces.dismiss();
  while (iters--) {
    CompactSerializer ser;
    IOBufQueue bufq;
    ser.serialize(data, &bufq);
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolReader_deserialize_shallow, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Shallow data = makeShallow();
  CompactSerializer ser;
  IOBufQueue bufq;
  ser.serialize(data, &bufq);
  auto buf = bufq.move();
  braces.dismiss();
  while (iters--) {
    CompactSerializer s;
    Shallow shallow;
    s.deserialize(buf.get(), shallow);
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolWriter_serialize_shallow, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Shallow data = makeShallow();
  braces.dismiss();
  while (iters--) {
    CompactSerializer ser;
    IOBufQueue bufq;
    ser.serialize(data, &bufq);
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolReader_deserialize_deep, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Deep data = makeDeep(16);
  CompactSerializer ser;
  IOBufQueue bufq;
  ser.serialize(data, &bufq);
  auto buf = bufq.move();
  braces.dismiss();
  while (iters--) {
    CompactSerializer s;
    Deep deep;
    s.deserialize(buf.get(), deep);
  }
  braces.rehire();
}

BENCHMARK(CompactProtocolWriter_serialize_deep, kiters) {
  BenchmarkSuspender braces;
  size_t iters = kiters << kMultExp;
  Deep data = makeDeep(16);
  braces.dismiss();
  while (iters--) {
    CompactSerializer ser;
    IOBufQueue bufq;
    ser.serialize(data, &bufq);
  }
  braces.rehire();
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  runBenchmarks();
  return 0;
}
