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
#ifndef incl_HPHP_ICU_MATCHER_H_
#define incl_HPHP_ICU_MATCHER_H_

#include <boost/scoped_ptr.hpp>

// Avoid dragging in the icu namespace.
#ifndef U_USING_ICU_NAMESPACE
#define U_USING_ICU_NAMESPACE 0
#endif

#include <unicode/regex.h>

namespace HPHP {
// Wrapper class around icu::RegexMatcher that provides a default constructor
// so that it can be used in thread-local storage.
// The easiest  way to use this as a thread-local variable is to make a small
// subclass with a fixed format string.
class ICUMatcher {
 public:
  ICUMatcher() : uStatus_(U_ZERO_ERROR) { }
  bool set(const icu::UnicodeString& pattern);
  bool matches(const icu::UnicodeString& word);
  icu::UnicodeString replaceAll(const icu::UnicodeString& word,
      const icu::UnicodeString& replacement);
 private:
  boost::scoped_ptr<icu::RegexMatcher> matcherPtr_;
  UErrorCode uStatus_;
};

}

#endif // incl_HPHP_ICU_MATCHER_H_
