/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
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

#include "hphp/util/struct-log.h"

#include <folly/Random.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

StructuredLogImpl StructuredLog::s_impl = nullptr;

bool StructuredLog::enabled() {
  return s_impl != nullptr;
}

bool StructuredLog::coinflip(uint32_t rate) {
  return enabled() && rate > 0 && folly::Random::rand32(rate) == 0;
}

void StructuredLog::enable(StructuredLogImpl impl) {
  s_impl = impl;
}

void StructuredLog::log(const std::string& tableName,
                        const StructuredLogEntry& cols) {
  if (enabled()) {
    s_impl(tableName, cols);
  }
}

///////////////////////////////////////////////////////////////////////////////

StructuredLogEntry::StructuredLogEntry()
  : ints(folly::dynamic::object()), strs(folly::dynamic::object())
{}

void StructuredLogEntry::setInt(folly::StringPiece key, int64_t value) {
  ints[key] = value;
}

void StructuredLogEntry::setStr(folly::StringPiece key,
                                folly::StringPiece value) {
  strs[key] = value;
}

void StructuredLogEntry::clear() {
  ints = folly::dynamic::object();
  strs = folly::dynamic::object();
}

std::string show(const StructuredLogEntry& cols) {
  folly::dynamic out = cols.strs;
  out["ints"] = cols.ints;
  return folly::toJson(out);
}

///////////////////////////////////////////////////////////////////////////////

}
