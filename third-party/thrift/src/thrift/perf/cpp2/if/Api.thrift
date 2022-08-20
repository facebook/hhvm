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

include "thrift/perf/cpp2/if/ApiBase.thrift"

namespace cpp2 facebook.thrift.benchmarks

service Benchmark {
  void noop() (thread = "eb");
  oneway void onewayNoop() (thread = "eb");
  ApiBase.TwoInts sum(1: ApiBase.TwoInts input) (thread = "eb");
  void timeout();
}
