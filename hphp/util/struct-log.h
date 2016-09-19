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

#ifndef incl_HPHP_UTIL_STRUCT_LOG_H_
#define incl_HPHP_UTIL_STRUCT_LOG_H_

#include <set>
#include <string>
#include <vector>
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
  void clear();

  folly::dynamic ints, strs, sets, vecs;
};

std::string show(const StructuredLogEntry&);

using StructuredLogImpl = void (*)(const std::string&,
                                   const StructuredLogEntry&);

/*
 * Interface for recording structured data for relatively infrequent events.
 */
namespace StructuredLog {
bool enabled();
bool coinflip(uint32_t rate);
void enable(StructuredLogImpl impl);
void log(const std::string& tableName, const StructuredLogEntry&);
};

///////////////////////////////////////////////////////////////////////////////

}

#endif
