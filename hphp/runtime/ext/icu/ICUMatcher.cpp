/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/icu/ICUMatcher.h"
#include "hphp/util/logger.h"
using namespace U_ICU_NAMESPACE;

namespace HPHP {

bool ICUMatcher::set(const UnicodeString& pattern) {
  matcherPtr_.reset(new icu::RegexMatcher(pattern, 0, uStatus_));
  if (U_FAILURE(uStatus_)) {
    Logger::Error("Error code: %s : Failed to create regex matcher.",
                  u_errorName(uStatus_));
    return false;
  }
  return true;
}

bool ICUMatcher::matches(const UnicodeString& word) {
  matcherPtr_->reset(word);
  return matcherPtr_->matches(uStatus_);
}

UnicodeString ICUMatcher::replaceAll(const UnicodeString& word,
    const UnicodeString& replacement) {
  matcherPtr_->reset(word);
  UnicodeString ret = matcherPtr_->replaceAll(replacement, uStatus_);
  return ret;
}

}
