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
#ifndef incl_HPHP_ICU_LIFE_EVENT_TOKENIZER_H_
#define incl_HPHP_ICU_LIFE_EVENT_TOKENIZER_H_

#include <atomic>
#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>

// Avoid dragging in the icu namespace.
#ifndef U_USING_ICU_NAMESPACE
#define U_USING_ICU_NAMESPACE 0
#endif

#include <unicode/rbbi.h> // icu

namespace HPHP {

typedef icu::RuleBasedBreakIterator BreakIterator;

// Master copy of the ICU object we use for tokenization. Creation is expensive,
// but cloning is fast and thread-safe, so we clone a copy (in tokenizeString)
// for each tokenization. Do NOT use it directly!
extern std::atomic<const BreakIterator*> kMaster;

// Most status values come from ICU's ubrk.h.  The following
// are Facebook custom.

const int32_t TOKEN_STATUS_EMAIL = 500;
const int32_t TOKEN_STATUS_URL = 501;
const int32_t TOKEN_STATUS_EMOTICON = 502;
const int32_t TOKEN_STATUS_HEART = 503;
const int32_t TOKEN_STATUS_EXCLAMATION = 504;
const int32_t TOKEN_STATUS_DATE = 505;
const int32_t TOKEN_STATUS_MONEY = 506;
const int32_t TOKEN_STATUS_TIME = 507;
const int32_t TOKEN_STATUS_ACRONYM = 508;

struct Token {
  Token(const icu::UnicodeString& v, int32_t s) : value(v), status(s) {}
  icu::UnicodeString value;
  int32_t status;
};

const BreakIterator* getMaster();

void tokenizeString(
  std::vector<Token>& tokenVectorOut,
  const BreakIterator* ptrBreakIterator,
  const icu::UnicodeString& ustr);

}

#endif // incl_HPHP_ICU_LIFE_EVENT_TOKENIZER_H_
