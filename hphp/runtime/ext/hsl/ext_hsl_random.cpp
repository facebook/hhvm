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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/random/ext_random.h"
#include "hphp/system/systemlib.h"
#include <folly/Random.h>

namespace HPHP {
namespace {

  RDS_LOCAL(folly::Random::DefaultGenerator, tl_rng);
  int64_t HHVM_FUNCTION(HH_pseudorandom_int, int64_t min, int64_t max) {
    return std::uniform_int_distribution<int64_t>(min, max)(*tl_rng);
  }

  void HHVM_FUNCTION(HH_pseudorandom_seed, int64_t seed) {
    tl_rng->seed(seed);
  }

  int64_t HHVM_FUNCTION(HH_random_int, int64_t min, int64_t max) {
    return getRandomInt(min, max);
  }

  struct RandomExtension final : Extension {
    RandomExtension() : Extension("hsl_random", "1.0", NO_ONCALL_YET) {}
    void moduleRegisterNative() override {
      // Clang 15 doesn't like the HHVM_FALIAS macro with \\N
      HHVM_FALIAS_FE_STR(
        "HH\\Lib\\_Private\\Native\\pseudorandom_int",
        HH_pseudorandom_int
      );
      HHVM_FALIAS_FE_STR(
        "HH\\Lib\\_Private\\Native\\pseudorandom_seed",
        HH_pseudorandom_seed
      );
      HHVM_FALIAS_FE_STR(
        "HH\\Lib\\_Private\\Native\\random_int",
        HH_random_int
      );
    }

    void requestInit() override {
      tl_rng->seed(folly::Random::secureRandom<int64_t>());
    }
  } s_random_extension;

} // anonymous namespace
} // namespace HPHP
