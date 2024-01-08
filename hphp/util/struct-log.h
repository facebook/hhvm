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

#pragma once

#include <set>
#include <string>
#include <vector>
#include <folly/futures/Future.h>
#include <folly/json.h>
#include <folly/Range.h>

namespace HPHP {

struct StackTrace;

///////////////////////////////////////////////////////////////////////////////

struct StructuredLogEntry {
  StructuredLogEntry();

  // Any previous value for the same key is silently overwritten.
  void setInt(folly::StringPiece key, int64_t value);
  void setStr(folly::StringPiece key, folly::StringPiece value);
  void setSet(folly::StringPiece key,
              const std::set<folly::StringPiece>& values);
  void setVec(folly::StringPiece key,
              const std::vector<folly::StringPiece>& values);
  void setStackTrace(folly::StringPiece key, const StackTrace& st);

  // Set the given key to a randomly generated UUID stable for this
  // process lifetime.
  void setProcessUuid(folly::StringPiece key);

  void clear();

  bool force_init{false};
  folly::dynamic ints, strs, sets, vecs;
};

std::string show(const StructuredLogEntry&);

/*
 * Interface for recording structured data for relatively infrequent events.
 */
namespace StructuredLog {
using LogFn = void (*)(const std::string&,
                       const StructuredLogEntry&);
using RecordGlobalsFn = void (*)(StructuredLogEntry&);

bool enabled();
bool coinflip(uint32_t rate);
void enable(LogFn log, RecordGlobalsFn globals);
void log(const std::string&, const StructuredLogEntry&);
void recordRequestGlobals(StructuredLogEntry&);
}

///////////////////////////////////////////////////////////////////////////////

}

