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

#include "hphp/util/stack-trace.h"

namespace HPHP {

StructuredLogEntry::StructuredLogEntry()
  : ints(folly::dynamic::object())
  , strs(folly::dynamic::object())
  , sets(folly::dynamic::object())
  , vecs(folly::dynamic::object())
{
}

void StructuredLogEntry::setInt(folly::StringPiece key, int64_t value) {
  ints[key] = value;
}

void StructuredLogEntry::setStr(folly::StringPiece key,
                                folly::StringPiece value) {
  strs[key] = value;
}

void StructuredLogEntry::setSet(folly::StringPiece key,
                                const std::set<folly::StringPiece>& value) {
  sets[key] = folly::dynamic::object();
  for (auto const& v : value) sets[key][v] = 1;
}

void StructuredLogEntry::setVec(folly::StringPiece key,
                                const std::vector<folly::StringPiece>& value) {
  folly::dynamic arr = folly::dynamic::array();
  arr.resize(value.size());
  for (int i = 0; i < value.size(); ++i) {
    arr[i] = value[i];
  }
  vecs[key] = arr;
}

void StructuredLogEntry::setStackTrace(folly::StringPiece key, StackTrace& st) {
  std::vector<folly::StringPiece> stackFrames;
  folly::split("\n", st.toString(), stackFrames);
  for (auto& frame : stackFrames) {
    const char* p = frame.begin();
    while (*p == '#' || *p == ' ' || (*p >= '0' && *p <= '9')) ++p;
    frame = folly::StringPiece(p, frame.end());
  }
  setVec(key, stackFrames);
}

void StructuredLogEntry::clear() {
  ints = folly::dynamic::object();
  strs = folly::dynamic::object();
  sets = folly::dynamic::object();
  vecs = folly::dynamic::object();
}

namespace StructuredLog {
namespace {
StructuredLogImpl s_impl = nullptr;
}

bool enabled() {
  return s_impl != nullptr;
}

bool coinflip(uint32_t rate) {
  return enabled() && rate > 0 && folly::Random::rand32(rate) == 0;
}

void enable(StructuredLogImpl impl) {
  s_impl = impl;
}

void log(const std::string& tableName, const StructuredLogEntry& cols) {
  if (enabled()) {
    s_impl(tableName, cols);
  }
}
}

std::string show(const StructuredLogEntry& cols) {
  folly::dynamic out = cols.strs;
  out["ints"] = cols.ints;
  out["sets"] = cols.sets;
  out["vecs"] = cols.vecs;
  std::ostringstream oss;
  oss << out;
  return oss.str();
}

}
