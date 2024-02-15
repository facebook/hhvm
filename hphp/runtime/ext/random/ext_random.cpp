/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/random/ext_random.h"

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/runtime.h"

#include <folly/Random.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static bool getRandomBytes(void *bytes, size_t length) {
  // TODO fix folly to better detect error cases (currently I think it just
  // aborts) and catch them here.
  folly::Random::secureRandom(bytes, length);
  return true;
}

String HHVM_FUNCTION(random_bytes, int64_t length) {
  if (length < 1) {
    SystemLib::throwErrorObject("Length must be greater than 0");
  }

  String ret(length, ReserveString);
  if (!getRandomBytes(ret.mutableData(), ret.capacity())) {
    SystemLib::throwErrorObject("Could not gather sufficient random data");
  }

  return ret.setSize(length);
}

int64_t getRandomInt(int64_t min, int64_t max) {
  if (min > max) {
    SystemLib::throwErrorObject(
      "Minimum value must be less than or equal to the maximum value");
  }

  if (min == max) {
    return min;
  }

  auto umax = static_cast<uint64_t>(max) - min;
  uint64_t result;
  if (!getRandomBytes(&result, sizeof(result))) {
    SystemLib::throwErrorObject("Could not gather sufficient random data");
  }

  // Special case where no modulus is required
  if (umax == std::numeric_limits<uint64_t>::max()) {
    return result;
  }

  // Increment the max so the range is inclusive of max
  umax++;

  // Powers of two are not biased
  if ((umax & (umax - 1)) != 0) {
    // Ceiling under which std::numeric_limits<uint64_t>::max() % max == 0
    int64_t limit = std::numeric_limits<uint64_t>::max() -
      (std::numeric_limits<uint64_t>::max() % umax) - 1;

    // Discard numbers over the limit to avoid modulo bias
    while (result > limit) {
      if (!getRandomBytes(&result, sizeof(result))) {
        SystemLib::throwErrorObject(
          "Could not gather sufficient random data");
      }
    }
  }

  return (int64_t)((result % umax) + min);
}

int64_t HHVM_FUNCTION(random_int, int64_t min, int64_t max) {
  return getRandomInt(min, max);
}

static struct RandomExtension final : Extension {
  RandomExtension() : Extension("random", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_FE(random_bytes);
    HHVM_FE(random_int);
  }
} s_random_extension;

HHVM_GET_MODULE(random);

///////////////////////////////////////////////////////////////////////////////
}
