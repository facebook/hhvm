/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/icu/ICUTransliterator.h"
#include "hphp/util/logger.h"

using namespace U_ICU_NAMESPACE;

namespace HPHP {

bool ICUTransliterator::set(const UnicodeString& transformId) {
  UParseError parseError;
  translitPtr_.reset(icu::Transliterator::createInstance(
        transformId,
        UTRANS_FORWARD,
        parseError,
        uStatus_));

  if(!translitPtr_ || U_FAILURE(uStatus_)) {
    Logger::Error("Error code: %s : Failed to initialize transliterator.",
                  u_errorName(uStatus_));
    return false;
  }
  return true;
}

void ICUTransliterator::transliterate(UnicodeString& s) {
  translitPtr_->transliterate(s);
}

}
