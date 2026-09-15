/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

// Benchmarks for the streaming zstd serializer. Run:
//   buck2 run @//mode/opt-hhvm fbcode//hphp/runtime/test/zstd-serialize-bench:zstd-serialize-bench

#include <string>

#include <folly/Benchmark.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/GFlags.h>
#include <zstd.h>

#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zstd-string-buffer.h"
#include "hphp/util/rds-local.h"

namespace HPHP {
namespace {

Variant makeValue(int n) {
  VecInit vi(n);
  for (int i = 0; i < n; i++) vi.append(make_tv<KindOfInt64>(i));
  return Variant{vi.toArray()};
}

// Built once and intentionally leaked: avoids per-iteration construction cost
// in the timed loop and avoids a request-heap dtor running after process exit.
const Variant& bigValue() {
  static const Variant* v = new Variant(makeValue(200000));  // ~1.8 MB
  return *v;
}

}  // namespace

//////////////////////////////////////////////////////////////////////////////

BENCHMARK(serialize_plain, iters) {
  for (auto i = iters; i > 0; --i) {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    vs.setIgnoreStringSizeLimit();
    auto s = vs.serialize(bigValue(), /*ret=*/true);
    folly::doNotOptimizeAway(s);
  }
}

BENCHMARK_RELATIVE(serialize_zstd_streaming, iters) {
  for (auto i = iters; i > 0; --i) {
    ZStdVariableSerializer vs(VariableSerializer::Type::Serialize);
    auto s = vs.serialize(bigValue(), /*ret=*/true);
    folly::doNotOptimizeAway(s);
  }
}

BENCHMARK_RELATIVE(serialize_then_zstd_oneshot, iters) {
  for (auto i = iters; i > 0; --i) {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    vs.setIgnoreStringSizeLimit();
    auto s = vs.serialize(bigValue(), /*ret=*/true);
    std::string out;
    out.resize(ZSTD_compressBound(s.size()));
    auto const n =
      ZSTD_compress(out.data(), out.size(), s.data(), s.size(), 6);
    folly::doNotOptimizeAway(n);
  }
}

}  // namespace HPHP

int main(int argc, char** argv) {
  // Parse gflags (for --bm_* flags) WITHOUT initializing glog: HHVM's
  // hphp_process_init initializes glog itself, and a second init aborts.
  folly::gflags::ParseCommandLineFlags(&argc, &argv, true);
  HPHP::rds::local::init();
  SCOPE_EXIT { HPHP::rds::local::fini(); };
  HPHP::init_for_unit_test();
  SCOPE_EXIT { HPHP::hphp_process_exit(); };
  folly::runBenchmarks();
  return 0;
}
