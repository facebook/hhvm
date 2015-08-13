/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/runtime.h"

#include <fstream>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// @TODO Make this a shared class in runtime/base/ dir?
// @TODO Throw exceptions instead of raising warnings
static bool getRandomBytes(void *bytes, size_t length) {
  // @TODO Check for sources of random at compile time
  // @TODO Add file pointer caching of some sort?
  std::ifstream randomSource ("/dev/urandom", std::ios::binary);
  if (!randomSource.is_open()) {
    raise_warning(
      "Cannot open source device"
    );
    return false;
  }

  randomSource.seekg(0, randomSource.beg);
  randomSource.read((char*)bytes, length);

  if (!randomSource) {
    raise_warning(
      "Could not gather sufficient random data"
    );
    randomSource.close();
		return false;
	}

  randomSource.close();

  return true;
}

Variant HHVM_FUNCTION(random_bytes, int64_t length) {
  if (length < 1) {
    raise_warning(
      "Length must be greater than 0"
    );
		return false;
	}

  char *bytes = (char*)calloc(length, 1);

  if (bytes == NULL) {
    raise_warning(
      "Cannot allocate memory"
    );
		return false;
  }

  if (!getRandomBytes(bytes, length)) {
    free(bytes);
    return false;
  }

  return String(bytes, length, AttachString);
}

Variant HHVM_FUNCTION(random_int, int64_t min, int64_t max) {
  if (min >= max) {
    raise_warning(
      "Minimum value must be less than the maximum value"
    );
		return false;
	}


	uint64_t umax = max - min;
  uint64_t result;
  if(!getRandomBytes(&result, sizeof(result))) {
    return false;
  }

  // Special case where no modulus is required
  // @TODO Replace literal with HHVM equivalent of ZEND_ULONG_MAX
	if (umax == 0xffffffffffffffff) {
    return result;
	}

	// Increment the max so the range is inclusive of max
	umax++;

  // Powers of two are not biased
	if ((umax & (umax - 1)) != 0) {
		// Ceiling under which 0xffffffffffffffff % max == 0
		int64_t limit = 0xffffffffffffffff - (0xffffffffffffffff % umax) - 1;

		// Discard numbers over the limit to avoid modulo bias
		while (result > limit) {
      if(!getRandomBytes(&result, sizeof(result))) {
        return false;
      }
		}
	}

  return (int64_t) ((result % umax) + min);
}

class RandomExtension final : public Extension {
public:
  RandomExtension() : Extension("random") {}
  void moduleInit() override {
    HHVM_FE(random_bytes);
    HHVM_FE(random_int);
    loadSystemlib();
  }
} s_random_extension;

HHVM_GET_MODULE(random);

///////////////////////////////////////////////////////////////////////////////
}
